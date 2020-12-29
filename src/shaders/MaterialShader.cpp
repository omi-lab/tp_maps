#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

/*
https://learnopengl.com/PBR/Theory

https://youtu.be/yn5UJzMqxj0
https://github.com/BennyQBD/3DEngineCpp/blob/master/res/shaders/sampling.glh

https://dominium.maksw.com/articles/physically-based-rendering-pbr/pbr-part-one/
https://www.chaosgroup.com/blog/understanding-metalness

GLSL Function Documantation
---------------------------
*/

//##################################################################################################
// Calculates GGX distribution.
/*
This creates the bright spot and soft edge of a specular highlight. I could not figure out what GGX
stands for it just seems to be a name.

The following are good references:
* http://www.neilblevins.com/cg_education/ggx/ggx.htm
* https://learnopengl.com/PBR/Theory
* https://dominium.maksw.com/articles/physically-based-rendering-pbr/pbr-part-one/

float calcGGXDist(vec3 norm, vec3 lightDirection_tangent, float roughness2);
*/

ShaderResource& vertShaderStr()       {static ShaderResource s{"/tp_maps/MaterialShader.render.vert"};  return s;}
ShaderResource& fragShaderStr()       {static ShaderResource s{"/tp_maps/MaterialShader.render.frag"};  return s;}
ShaderResource& vertShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.vert"}; return s;}
ShaderResource& fragShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.frag"}; return s;}
ShaderResource& vertShaderStrLight()  {static ShaderResource s{"/tp_maps/MaterialShader.light.vert"};   return s;}
ShaderResource& fragShaderStrLight()  {static ShaderResource s{"/tp_maps/MaterialShader.light.frag"};   return s;}

//##################################################################################################
struct LightLocations_lt
{
  // The matrix that transforms world coords onto the light texture.
  GLint worldToLightViewLocation{0};
  GLint worldToLightProjLocation{0};

  GLint         positionLocation{0};
  GLint        directionLocation{0};
  GLint          ambientLocation{0};
  GLint          diffuseLocation{0};
  GLint         specularLocation{0};
  GLint     diffuseScaleLocation{0};
  GLint         constantLocation{0};
  GLint           linearLocation{0};
  GLint        quadraticLocation{0};
  GLint      spotLightUVLocation{0};
  GLint      spotLightWHLocation{0};
  GLint             nearLocation{0};
  GLint              farLocation{0};

  GLint   lightTextureIDLocation{0};
};

//##################################################################################################
struct UniformLocations_lt
{
  GLint                     mMatrixLocation{0};
  GLint                    mvMatrixLocation{0};
  GLint                   mvpMatrixLocation{0};
  GLint                     vMatrixLocation{0};

  GLint                cameraOriginLocation{0};

  GLint          materialUseAmbientLocation{0};
  GLint          materialUseDiffuseLocation{0};
  GLint            materialUseNdotLLocation{0};
  GLint      materialUseAttenuationLocation{0};
  GLint           materialUseShadowLocation{0};
  GLint        materialUseLightMaskLocation{0};
  GLint       materialUseReflectionLocation{0};

  GLint         materialAlbedoScaleLocation{0};
  GLint       materialSpecularScaleLocation{0};

  GLint                     txlSizeLocation{0};
  GLint              discardOpacityLocation{0};

  GLint      rgbaTextureLocation{0};
  GLint  specularTextureLocation{0};
  GLint   normalsTextureLocation{0};
  GLint      rmaoTextureLocation{0};
  GLint spotLightTextureLocation{0};

  std::vector<LightLocations_lt> lightLocations;
};

//##################################################################################################
std::string replaceLight(const std::string& lightIndex, const std::string& levels, std::string result)
{
  auto replace = [&result](char key, const std::string& value)
  {
    size_t pos = result.find(key);
    while(pos != std::string::npos)
    {
      result.replace(pos, 1, value);
      pos = result.find(key, pos + value.size());
    }
  };

  replace('%', lightIndex);
  replace('@', levels);
  return result;
}
}

//##################################################################################################
struct MaterialShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::MaterialShader::Private");
  TP_NONCOPYABLE(Private);

  MaterialShader* q;

  ShaderType shaderType{ShaderType::Render};

  size_t maxLights{1};

  UniformLocations_lt renderLocations;
  UniformLocations_lt renderHDRLocations;

  GLint  pickingMVPMatrixLocation{0};
  GLint         pickingIDLocation{0};

  GLint    lightMVPMatrixLocation{0};

  GLuint emptyTextureID{0};
  GLuint emptyNormalTextureID{0};

  //################################################################################################
  Private(MaterialShader* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(q->map() && emptyTextureID)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(emptyTextureID);
      q->map()->deleteTexture(emptyNormalTextureID);
    }
  }

  //################################################################################################
  void draw(GLenum mode, MaterialShader::VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
MaterialShader::MaterialShader(Map* map, tp_maps::OpenGLProfile openGLProfile, bool compileShader):
  Geometry3DShader(map, openGLProfile),
  d(new Private(this))
{
  auto bindTexture = [&](const TPPixel& color)
  {
    tp_image_utils::ColorMap textureData{1, 1, nullptr, color};
    BasicTexture texture(map, textureData);
    return texture.bindTexture();
  };

  d->emptyTextureID       = bindTexture({  0,   0,   0, 255});
  d->emptyNormalTextureID = bindTexture({127, 127, 255, 255});

  if(compileShader)
  {
    auto compileRenderShader = [&](ShaderType shaderType)
    {
      std::string LIGHT_VERT_VARS;
      std::string LIGHT_VERT_CALC;

      std::string LIGHT_FRAG_VARS;
      std::string LIGHT_FRAG_CALC;

      {
        {
          //The number of lights we can used is limited by the number of available texture units.
          d->maxLights=0;
          GLint textureUnits=8;
          glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);

          //The number of textures used by the shader and the back buffer but excluding lights.
          int staticTextures = 5 + 1;
          if(textureUnits>staticTextures)
            d->maxLights = size_t(textureUnits) - size_t(staticTextures);
        }

        const auto& lights = map->lights();
        size_t iMax = tpMin(d->maxLights, lights.size());
        for(size_t i=0; i<iMax; i++)
        {
          const auto& light = lights.at(i);
          auto ii = std::to_string(i);

          size_t levels = (light.type==LightType::Spot)?map->spotLightLevels():1;
          auto ll = std::to_string(levels);

          LIGHT_VERT_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_view;\n");
          LIGHT_VERT_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_proj;\n");

          LIGHT_VERT_VARS += replaceLight(ii, ll, "/*TP_GLSL_OUT_V*/vec4 fragPos_light%View;\n\n");

          LIGHT_VERT_CALC += replaceLight(ii, ll, "  fragPos_light%View = worldToLight%_view * (m * vec4(inVertex, 1.0));\n");

          LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform vec3 light%Direction_world;\n");
          LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform Light light%;\n");
          LIGHT_FRAG_VARS += replaceLight(ii, ll, "/*TP_GLSL_IN_F*/vec4 fragPos_light%View;\n\n");
          LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_proj;\n");

          LIGHT_FRAG_CALC += "  {\n";
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    vec3 ldNormalized = normalize(invTBN * light%Direction_world);\n");

          switch(light.type)
          {
          case LightType::Global:[[fallthrough]];
          case LightType::Directional:
          {
            LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform sampler2D light%Texture;\n");
            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = directionalLight(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec4(0,0,0,0), worldToLight%_proj));\n");
            break;
          }

          case LightType::Spot:
          {
            if(map->spotLightLevels() == 1)
            {
              LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform sampler2D light%Texture;\n");
              LIGHT_FRAG_CALC += replaceLight(ii, ll, "    float shadow=0.0;\n");
              LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow += spotLightSampleShadow2D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec4(0,0,0,0), worldToLight%_proj));\n");
              //LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = spotLight2D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec4(0,0,0,0), worldToLight%_proj));\n");
              LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow /= totalSadowSamples;\n");
            }
            else
            {
              LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform sampler3D light%Texture;\n");
              LIGHT_FRAG_CALC += replaceLight(ii, ll, "    float shadow=0.0;\n");

              for(size_t l=0; l<levels; l++)
              {
                std::string levelTexCoord = std::to_string(float(l)/float(levels-1));

                auto o = Light::lightLevelOffsets()[l] * light.offsetScale;
                std::string offset = "vec4(" + std::to_string(o.x) + "," + std::to_string(o.y) + "," + std::to_string(o.z) + ",0.0)";

                LIGHT_FRAG_CALC += replaceLight(ii, std::to_string(l), "    shadow += spotLightSampleShadow3D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View,"+offset+", worldToLight%_proj), " + levelTexCoord + ");\n");
              }

              LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow /= totalSadowSamples * @.0;\n");
              //LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = spotLight3D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec4(0,0,0,0), worldToLight%_proj), shadow);\n");
            }

            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow = mix(1.0, shadow, material.useShadow);\n");
            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = spotLight(norm, light%, ldNormalized, lightPosToTexture(fragPos_light%View, vec4(0,0,0,0), worldToLight%_proj), shadow);\n");
            break;
          }
          }

          LIGHT_FRAG_CALC += "    ambient  += r.ambient;\n";
          LIGHT_FRAG_CALC += "    diffuse  += r.diffuse;\n";
          LIGHT_FRAG_CALC += "    specular += r.specular;\n";
          LIGHT_FRAG_CALC += "  }\n\n";
        }
      }

      LIGHT_VERT_VARS = parseShaderString(LIGHT_VERT_VARS, openGLProfile, shaderType);
      LIGHT_VERT_CALC = parseShaderString(LIGHT_VERT_CALC, openGLProfile, shaderType);
      LIGHT_FRAG_VARS = parseShaderString(LIGHT_FRAG_VARS, openGLProfile, shaderType);
      LIGHT_FRAG_CALC = parseShaderString(LIGHT_FRAG_CALC, openGLProfile, shaderType);

      std::string vertStr = vertShaderStr().data(openGLProfile, shaderType);
      std::string fragStr = fragShaderStr().data(openGLProfile, shaderType);

      auto replace = [&](std::string& result, const std::string& key, const std::string& value)
      {
        size_t pos = result.find(key);
        while(pos != std::string::npos)
        {
          result.replace(pos, key.size(), value);
          pos = result.find(key, pos + value.size());
        }
      };

      replace(vertStr, "/*LIGHT_VERT_VARS*/", LIGHT_VERT_VARS);
      replace(vertStr, "/*LIGHT_VERT_CALC*/", LIGHT_VERT_CALC);
      replace(fragStr, "/*LIGHT_FRAG_VARS*/", LIGHT_FRAG_VARS);
      replace(fragStr, "/*LIGHT_FRAG_CALC*/", LIGHT_FRAG_CALC);

      replace(fragStr, "/*TP_SHADOW_SAMPLES*/", std::to_string(map->shadowSamples()));

      compile(vertStr.c_str(), fragStr.c_str(), [](auto){}, [](auto){}, shaderType);
    };

    auto compileOtherShader = [&](ShaderResource& vertStr, ShaderResource& fragStr, ShaderType shaderType)
    {
      compile(vertStr.data(openGLProfile, shaderType), fragStr.data(openGLProfile, shaderType), [](auto){}, [](auto){}, shaderType);
    };

    compileRenderShader(ShaderType::Render);
    compileRenderShader(ShaderType::RenderHDR);
    compileOtherShader(vertShaderStrPicking(), fragShaderStrPicking(), ShaderType::Picking);
    compileOtherShader(vertShaderStrLight(), fragShaderStrLight(), ShaderType::Light);
  }
}

//##################################################################################################
MaterialShader::~MaterialShader()
{
  delete d;
}

//################################################################################################
void MaterialShader::compile(const char* vertShaderStr,
                             const char* fragShaderStr,
                             const std::function<void(GLuint)>& bindLocations,
                             const std::function<void(GLuint)>& getLocations,
                             ShaderType shaderType)
{
  Shader::compile(vertShaderStr,
                  fragShaderStr,
                  [&](GLuint program)
  {
    switch(shaderType)
    {
    case ShaderType::Render: [[fallthrough]];
    case ShaderType::RenderHDR:
    {
      glBindAttribLocation(program, 0, "inVertex");
      glBindAttribLocation(program, 1, "inNormal");
      glBindAttribLocation(program, 2, "inTangent");
      glBindAttribLocation(program, 3, "inBitangent");
      glBindAttribLocation(program, 4, "inTexture");
      bindLocations(program);
      break;
    }

    case ShaderType::Picking: [[fallthrough]];
    case ShaderType::Light:
    {
      glBindAttribLocation(program, 0, "inVertex");
      break;
    }
    }

  },
  [&](GLuint program)
  {
    auto exec = [&](UniformLocations_lt& locations)
    {
      locations.mMatrixLocation           = glGetUniformLocation(program, "m");
      locations.mvMatrixLocation          = glGetUniformLocation(program, "mv");
      locations.mvpMatrixLocation         = glGetUniformLocation(program, "mvp");
      locations.vMatrixLocation           = glGetUniformLocation(program, "v");

      locations.cameraOriginLocation      = glGetUniformLocation(program, "cameraOrigin_world");

      locations.    materialUseAmbientLocation = glGetUniformLocation(program, "material.useAmbient"    );
      locations.    materialUseDiffuseLocation = glGetUniformLocation(program, "material.useDiffuse"    );
      locations.      materialUseNdotLLocation = glGetUniformLocation(program, "material.useNdotL"      );
      locations.materialUseAttenuationLocation = glGetUniformLocation(program, "material.useAttenuation");
      locations.     materialUseShadowLocation = glGetUniformLocation(program, "material.useShadow"     );
      locations.  materialUseLightMaskLocation = glGetUniformLocation(program, "material.useLightMask"  );
      locations. materialUseReflectionLocation = glGetUniformLocation(program, "material.useReflection" );

      locations.  materialAlbedoScaleLocation = glGetUniformLocation(program, "material.albedoScale"  );
      locations.materialSpecularScaleLocation = glGetUniformLocation(program, "material.specularScale");

      locations.txlSizeLocation           = glGetUniformLocation(program, "txlSize");
      locations.discardOpacityLocation    = glGetUniformLocation(program, "discardOpacity");

      locations.     rgbaTextureLocation = glGetUniformLocation(program, "rgbaTexture"     );
      locations. specularTextureLocation = glGetUniformLocation(program, "specularTexture" );
      locations.  normalsTextureLocation = glGetUniformLocation(program, "normalsTexture"  );
      locations.     rmaoTextureLocation = glGetUniformLocation(program, "rmaoTexture"     );
      locations.spotLightTextureLocation = glGetUniformLocation(program, "spotLightTexture");

      const auto& lights = map()->lights();
      size_t iMax = tpMin(d->maxLights, lights.size());

      locations.lightLocations.resize(iMax);
      for(size_t i=0; i<locations.lightLocations.size(); i++)
      {
        auto& lightLocations = locations.lightLocations.at(i);

        auto ii = std::to_string(i);

        lightLocations.worldToLightViewLocation = glGetUniformLocation(program, replaceLight(ii, "", "worldToLight%_view").c_str());
        lightLocations.worldToLightProjLocation = glGetUniformLocation(program, replaceLight(ii, "", "worldToLight%_proj").c_str());

        lightLocations.positionLocation         = glGetUniformLocation(program, replaceLight(ii, "", "light%.position").c_str());
        lightLocations.directionLocation        = glGetUniformLocation(program, replaceLight(ii, "", "light%Direction_world").c_str());
        lightLocations.ambientLocation          = glGetUniformLocation(program, replaceLight(ii, "", "light%.ambient").c_str());
        lightLocations.diffuseLocation          = glGetUniformLocation(program, replaceLight(ii, "", "light%.diffuse").c_str());
        lightLocations.specularLocation         = glGetUniformLocation(program, replaceLight(ii, "", "light%.specular").c_str());
        lightLocations.diffuseScaleLocation     = glGetUniformLocation(program, replaceLight(ii, "", "light%.diffuseScale").c_str());

        lightLocations.constantLocation         = glGetUniformLocation(program, replaceLight(ii, "", "light%.constant").c_str());
        lightLocations.linearLocation           = glGetUniformLocation(program, replaceLight(ii, "", "light%.linear").c_str());
        lightLocations.quadraticLocation        = glGetUniformLocation(program, replaceLight(ii, "", "light%.quadratic").c_str());
        lightLocations.spotLightUVLocation      = glGetUniformLocation(program, replaceLight(ii, "", "light%.spotLightUV").c_str());
        lightLocations.spotLightWHLocation      = glGetUniformLocation(program, replaceLight(ii, "", "light%.spotLightWH").c_str());

        lightLocations.nearLocation             = glGetUniformLocation(program, replaceLight(ii, "", "light%.near").c_str());
        lightLocations.farLocation              = glGetUniformLocation(program, replaceLight(ii, "", "light%.far").c_str());

        lightLocations.lightTextureIDLocation   = glGetUniformLocation(program, replaceLight(ii, "", "light%Texture").c_str());
      }
    };

    switch(shaderType)
    {
    case ShaderType::Render:
    {
      exec(d->renderLocations);
      break;
    }

    case ShaderType::RenderHDR:
    {
      exec(d->renderHDRLocations);
      break;
    }

    case ShaderType::Picking:
    {
      d->pickingMVPMatrixLocation = glGetUniformLocation(program, "mvp");
      d->pickingIDLocation         = glGetUniformLocation(program, "pickingID");
      break;
    }

    case ShaderType::Light:
    {
      d->lightMVPMatrixLocation = glGetUniformLocation(program, "mvp");
      break;
    }
    }

    getLocations(program);
  },
  shaderType);
}

//##################################################################################################
void MaterialShader::use(ShaderType shaderType)
{
  d->shaderType = shaderType;

  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  // if(shaderType == ShaderType::RenderHDR)
  //   Shader::use(ShaderType::Render);
  // else
    Shader::use(shaderType);

}

//##################################################################################################
void MaterialShader::setMaterial(const Material& material)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f (locations.    materialUseAmbientLocation, material.useAmbient    );
    glUniform1f (locations.    materialUseDiffuseLocation, material.useDiffuse    );
    glUniform1f (locations.      materialUseNdotLLocation, material.useNdotL      );
    glUniform1f (locations.materialUseAttenuationLocation, material.useAttenuation);
    glUniform1f (locations.     materialUseShadowLocation, material.useShadow     );
    glUniform1f (locations.  materialUseLightMaskLocation, material.useLightMask  );
    glUniform1f (locations. materialUseReflectionLocation, material.useReflection );

    glUniform1f(locations.    materialAlbedoScaleLocation, material.albedoScale   );
    glUniform1f(locations.  materialSpecularScaleLocation, material.specularScale );
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);

  else if(d->shaderType == ShaderType::RenderHDR)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void MaterialShader::setLights(const std::vector<Light>& lights, const std::vector<FBO>& lightBuffers)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    {
      size_t iMax = tpMin(lights.size(), locations.lightLocations.size());
      for(size_t i=0; i<iMax; i++)
      {
        const auto& light = lights.at(i);
        const auto& lightLocations = locations.lightLocations.at(i);

        glm::vec3 position = light.position();
        glm::vec3 direction = light.direction();

        if(lightLocations. positionLocation>=0) glUniform3fv(lightLocations. positionLocation, 1,    &position.x       );
        if(lightLocations.directionLocation>=0) glUniform3fv(lightLocations.directionLocation, 1,  &direction.x        );
        if(lightLocations.  ambientLocation>=0) glUniform3fv(lightLocations.  ambientLocation, 1,  &light.ambient.x    );
        if(lightLocations.  diffuseLocation>=0) glUniform3fv(lightLocations.  diffuseLocation, 1,  &light.diffuse.x    );
        if(lightLocations. specularLocation>=0) glUniform3fv(lightLocations. specularLocation, 1,  &light.specular.x   );

        glUniform1f (lightLocations.diffuseScaleLocation,      light.diffuseScale );
        glUniform1f (lightLocations.    constantLocation,      light.constant     );
        glUniform1f (lightLocations.      linearLocation,      light.linear       );
        glUniform1f (lightLocations.   quadraticLocation,      light.quadratic    );
        glUniform2fv(lightLocations. spotLightUVLocation, 1,  &light.spotLightUV.x);
        glUniform2fv(lightLocations. spotLightWHLocation, 1,  &light.spotLightWH.x);
        glUniform1f (lightLocations.        nearLocation,      light.near         );
        glUniform1f (lightLocations.         farLocation,      light.far          );
      }
    }

    {
      size_t iMax = tpMin(lightBuffers.size(), locations.lightLocations.size());
      for(size_t i=0; i<iMax; i++)
      {
        const auto& lightBuffer = lightBuffers.at(i);
        const auto& lightLocations = locations.lightLocations.at(i);

        glUniformMatrix4fv(lightLocations.worldToLightViewLocation, 1, GL_FALSE, glm::value_ptr(lightBuffer.worldToTexture[0].v));
        glUniformMatrix4fv(lightLocations.worldToLightProjLocation, 1, GL_FALSE, glm::value_ptr(lightBuffer.worldToTexture[0].p));

        glActiveTexture(GLenum(GL_TEXTURE6 + i));

        if(lightBuffer.levels == 1)
          glBindTexture(GL_TEXTURE_2D, lightBuffer.depthID);
        else
          glBindTexture(GL_TEXTURE_3D, lightBuffer.depthID);

        glUniform1i(lightLocations.lightTextureIDLocation, GLint(6 + i));
      }
    }

    {
      glm::vec2 txlSize{1.0f, 1.0f};
      if(!lightBuffers.empty())
        txlSize = glm::vec2(1.0, 1.0) / glm::vec2(lightBuffers[0].width, lightBuffers[0].height);
      glUniform2fv(locations.txlSizeLocation, 1, &txlSize.x);
    }
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);
  else if(d->shaderType == ShaderType::RenderHDR)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void MaterialShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 mv = v*m;
  glm::mat4 mvp = p*v*m;

  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniformMatrix4fv(locations.mMatrixLocation  , 1, GL_FALSE, glm::value_ptr(m));
    glUniformMatrix4fv(locations.mvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mv));
    glUniformMatrix4fv(locations.mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniformMatrix4fv(locations.vMatrixLocation, 1, GL_FALSE, glm::value_ptr(v));

    if(locations.cameraOriginLocation>=0)
    {
      glm::vec3 cameraOrigin_world = glm::inverse(v) * glm::vec4(0,0,0,1);
      glUniform3fv(locations.cameraOriginLocation, 1, &cameraOrigin_world.x);
    }
  };

  switch(d->shaderType)
  {
  case ShaderType::Render:
  {
    exec(d->renderLocations);
    break;
  }

  case ShaderType::RenderHDR:
  {
    exec(d->renderHDRLocations);
    break;
  }

  case ShaderType::Picking:
  {
    glUniformMatrix4fv(d->pickingMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    break;
  }

  case ShaderType::Light:
  {
    glUniformMatrix4fv(d->lightMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    break;
  }
  }
}

//##################################################################################################
void MaterialShader::draw(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::drawPicking(GLenum mode,
                                 VertexBuffer* vertexBuffer,
                                 const glm::vec4& pickingID)
{
  glDisable(GL_BLEND);
  glUniform4fv(d->pickingIDLocation, 1, &pickingID.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::invalidate()
{
  d->emptyTextureID = 0;
  d->emptyNormalTextureID = 0;

  Geometry3DShader::invalidate();
}

//##################################################################################################
void MaterialShader::drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::setTextures(GLuint rgbaTextureID,
                                 GLuint specularTextureID,
                                 GLuint normalsTextureID,
                                 GLuint rmaoTextureID,
                                 GLuint spotLightTextureID)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbaTextureID);
    glUniform1i(locations.rgbaTextureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularTextureID);
    glUniform1i(locations.specularTextureLocation, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalsTextureID);
    glUniform1i(locations.normalsTextureLocation, 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, rmaoTextureID);
    glUniform1i(locations.rmaoTextureLocation, 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, spotLightTextureID);
    glUniform1i(locations.spotLightTextureLocation, 4);
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);
  else if(d->shaderType == ShaderType::RenderHDR)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void MaterialShader::setBlankTextures()
{
  setTextures(d->emptyTextureID,
              d->emptyTextureID,
              d->emptyNormalTextureID,
              d->emptyTextureID,
              map()->spotLightTexture());
}

//##################################################################################################
void MaterialShader::setDiscardOpacity(float discardOpacity)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f(locations.discardOpacityLocation, discardOpacity);
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);
  else if(d->shaderType == ShaderType::RenderHDR)
    exec(d->renderHDRLocations);
}

}
