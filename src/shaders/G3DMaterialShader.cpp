#include "tp_maps/shaders/G3DMaterialShader.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_math_utils/Material.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

/*
https://google.github.io/filament/Filament.html#materialsystem/clearcoatmodel

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

ShaderResource& vertShaderStr()       {static ShaderResource s{"/tp_maps/G3DMaterialShader.render.vert"};  return s;}
ShaderResource& fragShaderStr()       {static ShaderResource s{"/tp_maps/G3DMaterialShader.render.frag"};  return s;}
ShaderResource& vertShaderStrPicking(){static ShaderResource s{"/tp_maps/G3DMaterialShader.picking.vert"}; return s;}
ShaderResource& fragShaderStrPicking(){static ShaderResource s{"/tp_maps/G3DMaterialShader.picking.frag"}; return s;}
ShaderResource& vertShaderStrLight()  {static ShaderResource s{"/tp_maps/G3DMaterialShader.light.vert"};   return s;}
ShaderResource& fragShaderStrLight()  {static ShaderResource s{"/tp_maps/G3DMaterialShader.light.frag"};   return s;}

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
  GLint     diffuseScaleLocation{0};
  GLint         constantLocation{0};
  GLint           linearLocation{0};
  GLint        quadraticLocation{0};
  GLint   spotLightBlendLocation{0};
  GLint             nearLocation{0};
  GLint              farLocation{0};
  GLint      offsetScaleLocation{0};
  GLint              fovLocation{0};

  GLint   lightTextureIDLocation{0};
};

//##################################################################################################
struct UniformLocations_lt
{
  GLint                             mMatrixLocation{0};
  GLint                            mvMatrixLocation{0};
  GLint                           mvpMatrixLocation{0};
  GLint                             vMatrixLocation{0};
  GLint                          mInvMatrixLocation{0};
  GLint                            uvMatrixLocation{0};

  GLint                        cameraOriginLocation{0};

  GLint                  materialUseAmbientLocation{0};
  GLint                  materialUseDiffuseLocation{0};
  GLint                    materialUseNdotLLocation{0};
  GLint              materialUseAttenuationLocation{0};
  GLint                   materialUseShadowLocation{0};
  GLint                materialUseLightMaskLocation{0};
  GLint               materialUseReflectionLocation{0};

  GLint               materialShadowCatcherLocation{0};

  GLint                 materialAlbedoScaleLocation{0};


  GLint            materialAlbedoBrightnessLocation{0};
  GLint              materialAlbedoContrastLocation{0};
  GLint                 materialAlbedoGammaLocation{0};
  GLint                   materialAlbedoHueLocation{0};
  GLint            materialAlbedoSaturationLocation{0};
  GLint                 materialAlbedoValueLocation{0};
  GLint                materialAlbedoFactorLocation{0};


  GLint                             txlSizeLocation{0};
  GLint                       shadowSamplesLocation{0};
  GLint                      discardOpacityLocation{0};
  GLint                        lightOffsetsLocation{0};

  GLint                         rgbaTextureLocation{0};
  GLint                      normalsTextureLocation{0};
  GLint                        rmttrTextureLocation{0};

  std::vector<LightLocations_lt> lightLocations;
};

}

//##################################################################################################
struct G3DMaterialShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_math_utils::MaterialShader::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  size_t maxLights{1};

  UniformLocations_lt renderLocations;
  UniformLocations_lt renderHDRLocations;

  GLint  pickingMVPMatrixLocation{0};
  GLint         pickingIDLocation{0};

  GLint    lightMVPMatrixLocation{0};
  GLint     lightUVMatrixLocation{0};
  GLint  lightRGBATextureLocation{0};

  GLuint emptyTextureID{0};
  GLuint emptyNormalTextureID{0};

  //################################################################################################
  Private(Q* q_):
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
  void draw(GLenum mode, Geometry3DShader::VertexBuffer* vertexBuffer)
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

  //################################################################################################
  void compileRenderShader(ShaderType shaderType)
  {
    std::string LIGHT_VERT_VARS;
    std::string LIGHT_VERT_CALC;

    std::string LIGHT_FRAG_VARS;
    std::string LIGHT_FRAG_CALC;

    {
      {
        //The number of lights we can used is limited by the number of available texture units.
        maxLights=0;
        GLint textureUnits=8;
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);

        //The number of textures used by the shader and the back buffer but excluding lights.
        int staticTextures = 5 + 1;
        if(textureUnits>staticTextures)
          maxLights = size_t(textureUnits) - size_t(staticTextures);
      }

      LIGHT_FRAG_VARS += "uniform vec3 lightOffsets["+std::to_string(q->map()->maxSpotLightLevels())+"];\n";

      const auto& lights = q->map()->lights();
      size_t iMax = tpMin(maxLights, lights.size());
      for(size_t i=0; i<iMax; i++)
      {
        const auto& light = lights.at(i);
        auto ii = std::to_string(i);

        size_t levels = (light.type==tp_math_utils::LightType::Spot)?q->map()->maxSpotLightLevels():1;
        auto ll = std::to_string(levels);

        LIGHT_VERT_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_view;\n");
        LIGHT_VERT_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_proj;\n");

        LIGHT_VERT_VARS += replaceLight(ii, ll, "/*TP_GLSL_OUT_V*/vec4 fragPos_light%View;\n\n");

        LIGHT_VERT_CALC += replaceLight(ii, ll, "  fragPos_light%View = worldToLight%_view * (m * vec4(inVertex, 1.0));\n");

        LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform vec3 light%Direction_world;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform Light light%;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, ll, "/*TP_GLSL_IN_F*/vec4 fragPos_light%View;\n\n");
        LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform mat4 worldToLight%_proj;\n");

        LIGHT_FRAG_CALC += "\n  {\n";
        LIGHT_FRAG_CALC += replaceLight(ii, ll, "    vec3 ldNormalized;\n");

        LIGHT_FRAG_CALC += replaceLight(ii, ll, "    float shadow=0.0;\n");
        switch(light.type)
        {
        case tp_math_utils::LightType::Global:[[fallthrough]];
        case tp_math_utils::LightType::Directional:
        {
          LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform highp sampler2D light%Texture;\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    ldNormalized = normalize(invmTBN * light%Direction_world);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = directionalLight(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec2(0,0), worldToLight%_proj));\n");
          break;
        }

        case tp_math_utils::LightType::Spot:
        {
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    {\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "        vec4 a = worldToTangent * vec4(light%.position, 1.0);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "        ldNormalized = normalize(fragPos_tangent - a.xyz/a.w);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    }\n");

          if(q->map()->maxSpotLightLevels() == 1)
          {
            tpDebug() << "Spot light prepareBuffer()";
            LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform highp sampler2D light%Texture;\n");
            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow += spotLightSampleShadow2D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec2(0,0), worldToLight%_proj));\n");
            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow /= totShadowSamples();\n");
          }
          else
          {
            LIGHT_FRAG_VARS += replaceLight(ii, ll, "uniform sampler3D light%Texture;\n");
            LIGHT_FRAG_CALC += "    vec2 offset;\n";

            for (size_t levelIdx=0; levelIdx < levels; ++levelIdx)
            {
              LIGHT_FRAG_CALC += replaceLight(ii, std::to_string(levelIdx), "    offset = computeLightOffset(light%, @);\n");
              LIGHT_FRAG_CALC += replaceLight(ii, std::to_string(levelIdx), "    shadow += spotLightSampleShadow3D(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, offset, worldToLight%_proj), lightOffsets[@].z);\n");
            }

            LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow /= totShadowSamples() * @.0;\n");
          }

          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    shadow = mix(1.0, shadow, material.useShadow);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, ll, "    LightResult r = spotLight(norm, light%, ldNormalized, lightPosToTexture(fragPos_light%View, vec2(0,0), worldToLight%_proj), shadow);\n");
          break;
        }
        }

        LIGHT_FRAG_CALC += "    ambient  += r.ambient;\n";
        LIGHT_FRAG_CALC += "    diffuse  += r.diffuse;\n";
        LIGHT_FRAG_CALC += "    specular += r.specular;\n";
        LIGHT_FRAG_CALC += "    accumulatedShadow *= shadow;\n";
        LIGHT_FRAG_CALC += "    numShadows += 1.0;\n";
        LIGHT_FRAG_CALC += "  }\n";
      }
    }

    LIGHT_VERT_VARS = parseShaderString(LIGHT_VERT_VARS, q->openGLProfile(), shaderType);
    LIGHT_VERT_CALC = parseShaderString(LIGHT_VERT_CALC, q->openGLProfile(), shaderType);
    LIGHT_FRAG_VARS = parseShaderString(LIGHT_FRAG_VARS, q->openGLProfile(), shaderType);
    LIGHT_FRAG_CALC = parseShaderString(LIGHT_FRAG_CALC, q->openGLProfile(), shaderType);

    vertSrcScratch = vertShaderStr().data(q->openGLProfile(), shaderType);
    fragSrcScratch = fragShaderStr().data(q->openGLProfile(), shaderType);

    tp_utils::replace(vertSrcScratch, "/*LIGHT_VERT_VARS*/", LIGHT_VERT_VARS);
    tp_utils::replace(vertSrcScratch, "/*LIGHT_VERT_CALC*/", LIGHT_VERT_CALC);
    tp_utils::replace(fragSrcScratch, "/*LIGHT_FRAG_VARS*/", LIGHT_FRAG_VARS);
    tp_utils::replace(fragSrcScratch, "/*LIGHT_FRAG_CALC*/", LIGHT_FRAG_CALC);
  }

  //################################################################################################
  void compileOtherShader(ShaderResource& vertShaderStr, ShaderResource& fragShaderStr, ShaderType shaderType)
  {
    vertSrcScratch = vertShaderStr.data(q->openGLProfile(), shaderType);
    fragSrcScratch = fragShaderStr.data(q->openGLProfile(), shaderType);
  };

};

//##################################################################################################
G3DMaterialShader::G3DMaterialShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
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
}

//##################################################################################################
G3DMaterialShader::~G3DMaterialShader()
{
  delete d;
}



//##################################################################################################
void G3DMaterialShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}

//##################################################################################################
void G3DMaterialShader::setLights(const std::vector<tp_math_utils::Light>& lights,
                                  const std::vector<FBO>& lightBuffers)
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

        if(lightLocations.   positionLocation>=0)glUniform3fv(lightLocations.   positionLocation, 1, &position.x         );
        if(lightLocations.  directionLocation>=0)glUniform3fv(lightLocations.  directionLocation, 1, &direction.x        );
        if(lightLocations.    ambientLocation>=0)glUniform3fv(lightLocations.    ambientLocation, 1, &light.ambient.x    );
        if(lightLocations.    diffuseLocation>=0)glUniform3fv(lightLocations.    diffuseLocation, 1, &light.diffuse.x    );
        if(lightLocations.offsetScaleLocation>=0)glUniform3fv(lightLocations.offsetScaleLocation, 1, &light.offsetScale.x);

        glUniform1f(lightLocations.  diffuseScaleLocation, light.diffuseScale  );
        glUniform1f(lightLocations.      constantLocation, light.constant      );
        glUniform1f(lightLocations.        linearLocation, light.linear        );
        glUniform1f(lightLocations.     quadraticLocation, light.quadratic     );
        glUniform1f(lightLocations.spotLightBlendLocation, light.spotLightBlend);
        glUniform1f(lightLocations.          nearLocation, light.near          );
        glUniform1f(lightLocations.           farLocation, light.far           );
        glUniform1f(lightLocations.           fovLocation, glm::radians(light.fov));
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

    setShadowSamples(map()->fastRender() ? map()->shadowSamplesFastRender()  : map()->shadowSamples());
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void G3DMaterialShader::setLightOffsets(size_t renderedlightLevels)
{
  if(renderedlightLevels<1)
    renderedlightLevels=1;

  auto exec = [&](const UniformLocations_lt& locations)
  {
    size_t lightLevels = map()->maxSpotLightLevels();

    std::vector<glm::vec3> lightOffsets;
    lightOffsets.reserve(lightLevels);
    for (size_t levelIndex = 0; levelIndex < lightLevels; ++levelIndex)
    {
      // If the light level hasn't been rendered, re-use an existing light level.
      size_t availableLevelIdx = levelIndex % renderedlightLevels;
      float levelTexCoord = float(availableLevelIdx)/float(lightLevels-1);
      lightOffsets.emplace_back(glm::vec3(tp_math_utils::Light::lightLevelOffsets()[availableLevelIdx], levelTexCoord));
    }
    glUniform3fv(locations.lightOffsetsLocation, GLsizei(lightLevels), glm::value_ptr(lightOffsets[0]));
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void G3DMaterialShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 mv = v*m;
  glm::mat4 mvp = p*v*m;
  glm::mat4 mInv = glm::inverse(m);

  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniformMatrix4fv(locations.   mMatrixLocation, 1, GL_FALSE, glm::value_ptr(m   ));
    glUniformMatrix4fv(locations.  mvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mv  ));
    glUniformMatrix4fv(locations. mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp ));
    glUniformMatrix4fv(locations.   vMatrixLocation, 1, GL_FALSE, glm::value_ptr(v   ));
    glUniformMatrix4fv(locations.mInvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mInv));

    if(locations.cameraOriginLocation>=0)
    {
      glm::vec3 cameraOrigin_world = glm::inverse(v) * glm::vec4(0,0,0,1);
      glUniform3fv(locations.cameraOriginLocation, 1, &cameraOrigin_world.x);
    }
  };

  switch(currentShaderType())
  {
  case ShaderType::Render:
  {
    exec(d->renderLocations);
    break;
  }

  case ShaderType::RenderExtendedFBO:
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
void G3DMaterialShader::setMaterial(const tp_math_utils::Material& material)
{
  setMaterial(material, material.uvTransformation.uvMatrix());
}

//##################################################################################################
void G3DMaterialShader::setMaterial(const tp_math_utils::Material& material, const glm::mat3& uvMatrix)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f (locations.    materialUseAmbientLocation, material.useAmbient                );
    glUniform1f (locations.    materialUseDiffuseLocation, material.useDiffuse                );
    glUniform1f (locations.      materialUseNdotLLocation, material.useNdotL                  );
    glUniform1f (locations.materialUseAttenuationLocation, material.useAttenuation            );
    glUniform1f (locations.     materialUseShadowLocation, material.useShadow                 );
    glUniform1f (locations.  materialUseLightMaskLocation, material.useLightMask              );
    glUniform1f (locations. materialUseReflectionLocation, material.useReflection             );

    glUniform1i (locations. materialShadowCatcherLocation, material.rayVisibilityShadowCatcher);

    glUniform1f(locations.    materialAlbedoScaleLocation, material.albedoScale               );
    glUniform1f(locations.    materialAlbedoBrightnessLocation, material.albedoBrightness     );
    glUniform1f(locations.    materialAlbedoContrastLocation  , material.albedoContrast       );
    glUniform1f(locations.    materialAlbedoGammaLocation     , material.albedoGamma          );
    glUniform1f(locations.    materialAlbedoHueLocation       , material.albedoHue            );
    glUniform1f(locations.    materialAlbedoSaturationLocation, material.albedoSaturation     );
    glUniform1f(locations.    materialAlbedoValueLocation     , material.albedoValue          );
    glUniform1f(locations.    materialAlbedoFactorLocation    , material.albedoFactor         );

    glUniformMatrix3fv(locations.uvMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);

  else if(currentShaderType() == ShaderType::Light)
    glUniformMatrix3fv(d->lightUVMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));
}

//##################################################################################################
void G3DMaterialShader::compile(ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render:
  case ShaderType::RenderExtendedFBO:
  d->compileRenderShader(shaderType);
  break;

  case ShaderType::Picking:
  d->compileOtherShader(vertShaderStrPicking(), fragShaderStrPicking(), shaderType);
  break;

  case ShaderType::Light:
  d->compileOtherShader(vertShaderStrLight(), fragShaderStrLight(), shaderType);
  break;
  }

  Geometry3DShader::compile(shaderType);
}

//##################################################################################################
const char* G3DMaterialShader::vertexShaderStr(ShaderType shaderType)
{
  TP_UNUSED(shaderType);
  return vertSrcScratch.c_str();
}

//##################################################################################################
const char* G3DMaterialShader::fragmentShaderStr(ShaderType shaderType)
{
  TP_UNUSED(shaderType);
  return fragSrcScratch.c_str();
}

//##################################################################################################
void G3DMaterialShader::bindLocations(GLuint program, ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inTBNq");
    glBindAttribLocation(program, 2, "inTexture");
    break;
  }

  case ShaderType::Picking:
  {
    glBindAttribLocation(program, 0, "inVertex");
    break;
  }
  case ShaderType::Light:
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 2, "inTexture");
    break;
  }
  }
}

//##################################################################################################
void G3DMaterialShader::getLocations(GLuint program, ShaderType shaderType)
{
  auto exec = [&](UniformLocations_lt& locations)
  {
    auto loc = [](auto program, const auto* name){return glGetUniformLocation(program, name);};

    locations.mMatrixLocation                = loc(program, "m");
    locations.mvMatrixLocation               = loc(program, "mv");
    locations.mvpMatrixLocation              = loc(program, "mvp");
    locations.vMatrixLocation                = loc(program, "v");
    locations.mInvMatrixLocation             = loc(program, "mInv");
    locations.uvMatrixLocation               = loc(program, "uvMatrix");

    locations.cameraOriginLocation           = loc(program, "cameraOrigin_world");

    locations.    materialUseAmbientLocation = loc(program, "material.useAmbient"    );
    locations.    materialUseDiffuseLocation = loc(program, "material.useDiffuse"    );
    locations.      materialUseNdotLLocation = loc(program, "material.useNdotL"      );
    locations.materialUseAttenuationLocation = loc(program, "material.useAttenuation");
    locations.     materialUseShadowLocation = loc(program, "material.useShadow"     );
    locations.  materialUseLightMaskLocation = loc(program, "material.useLightMask"  );
    locations. materialUseReflectionLocation = loc(program, "material.useReflection" );

    locations.materialShadowCatcherLocation  = loc(program, "material.rayVisibilityShadowCatcher");

    locations.  materialAlbedoScaleLocation      = loc(program, "material.albedoScale"  );
    locations.  materialAlbedoBrightnessLocation = loc(program, "material.albedoBrightness"  );
    locations.  materialAlbedoContrastLocation   = loc(program, "material.albedoContrast"    );
    locations.  materialAlbedoGammaLocation      = loc(program, "material.albedoGamma"       );
    locations.  materialAlbedoHueLocation        = loc(program, "material.albedoHue"         );
    locations.  materialAlbedoSaturationLocation = loc(program, "material.albedoSaturation"  );
    locations.  materialAlbedoValueLocation      = loc(program, "material.albedoValue"       );
    locations.  materialAlbedoFactorLocation     = loc(program, "material.albedoFactor"      );

    locations.txlSizeLocation                = loc(program, "txlSize");
    locations.shadowSamplesLocation          = loc(program, "shadowSamples");
    locations.discardOpacityLocation         = loc(program, "discardOpacity");
    locations.lightOffsetsLocation           = loc(program, "lightOffsets");

    locations.     rgbaTextureLocation       = loc(program, "rgbaTexture"     );
    locations.  normalsTextureLocation       = loc(program, "normalsTexture"  );
    locations.    rmttrTextureLocation       = loc(program, "rmttrTexture"    );

    const auto& lights = map()->lights();
    size_t iMax = tpMin(d->maxLights, lights.size());

    locations.lightLocations.resize(iMax);
    for(size_t i=0; i<locations.lightLocations.size(); i++)
    {
      auto& lightLocations = locations.lightLocations.at(i);

      auto ii = std::to_string(i);

      lightLocations.worldToLightViewLocation = loc(program, replaceLight(ii, "", "worldToLight%_view").c_str());
      lightLocations.worldToLightProjLocation = loc(program, replaceLight(ii, "", "worldToLight%_proj").c_str());

      lightLocations.positionLocation         = loc(program, replaceLight(ii, "", "light%.position").c_str());
      lightLocations.directionLocation        = loc(program, replaceLight(ii, "", "light%Direction_world").c_str());
      lightLocations.ambientLocation          = loc(program, replaceLight(ii, "", "light%.ambient").c_str());
      lightLocations.diffuseLocation          = loc(program, replaceLight(ii, "", "light%.diffuse").c_str());
      lightLocations.diffuseScaleLocation     = loc(program, replaceLight(ii, "", "light%.diffuseScale").c_str());

      lightLocations.constantLocation         = loc(program, replaceLight(ii, "", "light%.constant").c_str());
      lightLocations.linearLocation           = loc(program, replaceLight(ii, "", "light%.linear").c_str());
      lightLocations.quadraticLocation        = loc(program, replaceLight(ii, "", "light%.quadratic").c_str());
      lightLocations.spotLightBlendLocation   = loc(program, replaceLight(ii, "", "light%.spotLightBlend").c_str());

      lightLocations.nearLocation             = loc(program, replaceLight(ii, "", "light%.near").c_str());
      lightLocations.farLocation              = loc(program, replaceLight(ii, "", "light%.far").c_str());

      lightLocations.offsetScaleLocation      = loc(program, replaceLight(ii, "", "light%.offsetScale").c_str());

      lightLocations.fovLocation              = loc(program, replaceLight(ii, "", "light%.fov").c_str());
      
      lightLocations.lightTextureIDLocation   = loc(program, replaceLight(ii, "", "light%Texture").c_str());
    }
  };

  switch(shaderType)
  {
  case ShaderType::Render:
  {
    exec(d->renderLocations);
    break;
  }

  case ShaderType::RenderExtendedFBO:
  {
    exec(d->renderHDRLocations);
    break;
  }

  case ShaderType::Picking:
  {
    d->pickingMVPMatrixLocation = glGetUniformLocation(program, "mvp");
    d->pickingIDLocation        = glGetUniformLocation(program, "pickingID");
    break;
  }

  case ShaderType::Light:
  {
    d->lightMVPMatrixLocation = glGetUniformLocation(program, "mvp");
    d->lightUVMatrixLocation    = glGetUniformLocation(program, "uvMatrix");
    d->lightRGBATextureLocation = glGetUniformLocation(program, "rgbaTexture");
    break;
  }
  }
}

//##################################################################################################
void G3DMaterialShader::invalidate()
{
  d->emptyTextureID = 0;
  d->emptyNormalTextureID = 0;

  Geometry3DShader::invalidate();
}

//##################################################################################################
void G3DMaterialShader::drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DMaterialShader::setTextures(GLuint rgbaTextureID,
                                    GLuint normalsTextureID,
                                    GLuint rmttrTextureID)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbaTextureID);
    glUniform1i(locations.rgbaTextureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalsTextureID);
    glUniform1i(locations.normalsTextureLocation, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rmttrTextureID);
    glUniform1i(locations.rmttrTextureLocation, 2);
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);

  else if(currentShaderType() == ShaderType::Light)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbaTextureID);
    glUniform1i(d->lightRGBATextureLocation, 0);
  }
}

//##################################################################################################
void G3DMaterialShader::setBlankTextures()
{
  setTextures(d->emptyTextureID,
              d->emptyNormalTextureID,
              d->emptyTextureID);
}

//##################################################################################################
void G3DMaterialShader::setShadowSamples(int shadowSamples)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1i(locations.shadowSamplesLocation, shadowSamples);
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void G3DMaterialShader::setDiscardOpacity(float discardOpacity)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f(locations.discardOpacityLocation, discardOpacity);
  };

  if(currentShaderType() == ShaderType::Render)
    exec(d->renderLocations);

  else if(currentShaderType() == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void G3DMaterialShader::draw(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DMaterialShader::drawPicking(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& pickingID)
{
  glDisable(GL_BLEND);
  glUniform4fv(d->pickingIDLocation, 1, &pickingID.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DMaterialShader::initPass(RenderInfo& renderInfo,
                                 const Matrices& m,
                                 const glm::mat4& modelToWorldMatrix)
{
  use(renderInfo.shaderType());
  setMatrix(modelToWorldMatrix, m.v, m.p);
  setLights(map()->lights(), map()->lightBuffers());
  setLightOffsets(map()->renderedLightLevels());
}

//##################################################################################################
void G3DMaterialShader::setMaterial(RenderInfo& renderInfo,
                                    const ProcessedGeometry3D& processedGeometry3D)
{
  const auto& material = processedGeometry3D.alternativeMaterial->material;
  glm::mat3 uvMatrix = processedGeometry3D.uvMatrix * material.uvTransformation.uvMatrix();

  setMaterial(processedGeometry3D.alternativeMaterial->material, uvMatrix);

  setTextures(processedGeometry3D.alternativeMaterial->rgbaTextureID,
              processedGeometry3D.alternativeMaterial->normalsTextureID,
              processedGeometry3D.alternativeMaterial->rmttrTextureID);

  setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
}

//##################################################################################################
void G3DMaterialShader::setMaterialPicking(RenderInfo& renderInfo,
                                           const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void G3DMaterialShader::draw(RenderInfo& renderInfo,
                             const ProcessedGeometry3D& processedGeometry3D,
                             GLenum mode,
                             VertexBuffer* vertexBuffer)
{
  if(renderInfo.pass == RenderPass::LightFBOs &&
     processedGeometry3D.alternativeMaterial->material.rayVisibilityShadowCatcher)
    return;

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DMaterialShader::drawPicking(RenderInfo& renderInfo,
                                    const ProcessedGeometry3D& processedGeometry3D,
                                    GLenum mode,
                                    VertexBuffer* vertexBuffer,
                                    const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);

  if(processedGeometry3D.alternativeMaterial->material.rayVisibilityShadowCatcher)
    return;

  drawPicking(mode, vertexBuffer, pickingID);
}

}
