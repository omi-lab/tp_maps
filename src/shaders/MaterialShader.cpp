#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderResource& vertShaderStr()       {static ShaderResource s{"/tp_maps/MaterialShader.vert"};         return s;}
ShaderResource& fragShaderStr()       {static ShaderResource s{"/tp_maps/MaterialShader.frag"};         return s;}
ShaderResource& vertShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.vert"}; return s;}
ShaderResource& fragShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.frag"}; return s;}
ShaderResource& vertShaderStrLight()  {static ShaderResource s{"/tp_maps/MaterialShader.light.vert"};   return s;}
ShaderResource& fragShaderStrLight()  {static ShaderResource s{"/tp_maps/MaterialShader.light.frag"};   return s;}

//##################################################################################################
struct LightLocations_lt
{
  GLint worldToLightLocation    {0}; // The matrix that transforms world coords onto the light texture.

  GLint positionLocation        {0};
  GLint directionLocation       {0};
  GLint ambientLocation         {0};
  GLint diffuseLocation         {0};
  GLint specularLocation        {0};
  GLint diffuseScaleLocation    {0};
  GLint diffuseTranslateLocation{0};
  GLint constantLocation        {0};
  GLint linearLocation          {0};
  GLint quadraticLocation       {0};
  GLint spotLightUVLocation     {0};
  GLint spotLightWHLocation     {0};

  GLint lightTextureIDLocation  {0};
  GLint lightTextureSizeLocation{0};
};

//##################################################################################################
std::string replaceLight(const std::string& ii, std::string result)
{
  size_t pos = result.find('%');
  while(pos != std::string::npos)
  {
    result.replace(pos, 1, ii);
    pos = result.find('%', pos + ii.size());
  }
  return result;
}
}

//##################################################################################################
struct MaterialShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::MaterialShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  ShaderType shaderType{ShaderType::Render};

  size_t maxLights{1};

  GLint mMatrixLocation          {0};
  GLint mvpMatrixLocation        {0};

  GLint cameraOriginLocation     {0};

  GLint materialAmbientLocation  {0};
  GLint materialDiffuseLocation  {0};
  GLint materialSpecularLocation {0};
  GLint materialShininessLocation{0};
  GLint materialAlphaLocation    {0};

  GLint ambientTextureLocation   {0};
  GLint diffuseTextureLocation   {0};
  GLint specularTextureLocation  {0};
  //GLint alphaTextureLocation   {0};
  GLint bumpTextureLocation      {0};
  GLint spotLightTextureLocation {0};

  GLint pickingMVPMatrixLocation {0};
  GLint pickingIDLocation        {0};

  GLint lightMVPMatrixLocation   {0};

  std::vector<LightLocations_lt> lightLocations;

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
  d(new Private())
{
  if(compileShader)
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

        LIGHT_VERT_VARS += replaceLight(ii, "uniform mat4 worldToLight%;\n");
        LIGHT_VERT_VARS += replaceLight(ii, "uniform vec3 light%Direction_world;\n");
        LIGHT_VERT_VARS += replaceLight(ii, "/*TP_GLSL_OUT_V*/vec3 light%Direction_tangent;\n");
        LIGHT_VERT_VARS += replaceLight(ii, "/*TP_GLSL_OUT_V*/vec4 fragPos_light%;\n\n");

        LIGHT_VERT_CALC += replaceLight(ii, "  fragPos_light% = (worldToLight% * m) * vec4(inVertex, 1.0);\n");
        LIGHT_VERT_CALC += replaceLight(ii, "  light%Direction_tangent = TBN * light%Direction_world;\n\n");

        LIGHT_FRAG_VARS += replaceLight(ii, "uniform sampler2D light%Texture;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, "uniform vec2 light%TextureSize;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, "/*TP_GLSL_IN_F*/vec3 light%Direction_tangent;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, "uniform Light light%;\n");
        LIGHT_FRAG_VARS += replaceLight(ii, "/*TP_GLSL_IN_F*/vec4 fragPos_light%;\n\n");

        LIGHT_FRAG_CALC += "  {\n";
        switch(light.type)
        {
        case LightType::Global:[[fallthrough]];
        case LightType::Directional:
          LIGHT_FRAG_CALC += replaceLight(ii, "    vec3 fp = fragPos_light%.xyz;\n");
          LIGHT_FRAG_CALC += replaceLight(ii, "    fp = (vec3(0.5, 0.5, 0.5) * fp) + vec3(0.5, 0.5, 0.5);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, "    LightResult r = directionalLight(norm, light%, light%Direction_tangent, light%Texture, light%TextureSize, vec4(fp, 1));\n");
          break;

        case LightType::Spot:
          LIGHT_FRAG_CALC += replaceLight(ii, "    vec3 fp = fragPos_light%.xyz / fragPos_light%.w;\n");
          LIGHT_FRAG_CALC += replaceLight(ii, "    fp = (vec3(0.5, 0.5, 0.5) * fp) + vec3(0.5, 0.5, 0.5);\n");
          LIGHT_FRAG_CALC += replaceLight(ii, "    LightResult r = spotLight(norm, light%, light%Direction_tangent, light%Texture, light%TextureSize, vec4(fp, 1));\n");
          break;
        }

        LIGHT_FRAG_CALC += "    ambient  += r.ambient;\n";
        LIGHT_FRAG_CALC += "    diffuse  += r.diffuse;\n";
        LIGHT_FRAG_CALC += "    specular += r.specular;\n";
        LIGHT_FRAG_CALC += "  }\n\n";
      }
    }

    LIGHT_VERT_VARS = parseShaderString(LIGHT_VERT_VARS, openGLProfile);
    LIGHT_VERT_CALC = parseShaderString(LIGHT_VERT_CALC, openGLProfile);
    LIGHT_FRAG_VARS = parseShaderString(LIGHT_FRAG_VARS, openGLProfile);
    LIGHT_FRAG_CALC = parseShaderString(LIGHT_FRAG_CALC, openGLProfile);

    std::string vertStr = vertShaderStr().data(openGLProfile);
    std::string fragStr = fragShaderStr().data(openGLProfile);

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

    compile(vertStr.c_str(), fragStr.c_str(), [](auto){}, [](auto){});
    compile(vertShaderStrPicking().data(openGLProfile), fragShaderStrPicking().data(openGLProfile), [](auto){}, [](auto){}, ShaderType::Picking);
    compile(vertShaderStrLight().data(openGLProfile), fragShaderStrLight().data(openGLProfile), [](auto){}, [](auto){}, ShaderType::Light);
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
    case ShaderType::Render:
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
    switch(shaderType)
    {
    case ShaderType::Render:
    {
      d->mMatrixLocation           = glGetUniformLocation(program, "m");
      d->mvpMatrixLocation         = glGetUniformLocation(program, "mvp");

      d->cameraOriginLocation      = glGetUniformLocation(program, "cameraOrigin_world");

      d->materialAmbientLocation   = glGetUniformLocation(program, "material.ambient");
      d->materialDiffuseLocation   = glGetUniformLocation(program, "material.diffuse");
      d->materialSpecularLocation  = glGetUniformLocation(program, "material.specular");
      d->materialShininessLocation = glGetUniformLocation(program, "material.shininess");
      d->materialAlphaLocation     = glGetUniformLocation(program, "material.alpha");

      d->ambientTextureLocation   = glGetUniformLocation(program, "ambientTexture");
      d->diffuseTextureLocation   = glGetUniformLocation(program, "diffuseTexture");
      d->specularTextureLocation  = glGetUniformLocation(program, "specularTexture");
      //d->alphaTextureIDLocation = glGetUniformLocation(program, "ambientTexture");
      d->bumpTextureLocation      = glGetUniformLocation(program, "bumpTexture");
      d->spotLightTextureLocation = glGetUniformLocation(program, "spotLightTexture");

      const auto& lights = map()->lights();
      size_t iMax = tpMin(d->maxLights, lights.size());

      d->lightLocations.resize(iMax);
      for(size_t i=0; i<d->lightLocations.size(); i++)
      {
        auto& lightLocations = d->lightLocations.at(i);

        auto ii = std::to_string(i);

        lightLocations.worldToLightLocation     = glGetUniformLocation(program, replaceLight(ii, "worldToLight%").c_str());

        lightLocations.positionLocation         = glGetUniformLocation(program, replaceLight(ii, "light%.position").c_str());
        lightLocations.directionLocation        = glGetUniformLocation(program, replaceLight(ii, "light%Direction_world").c_str());
        lightLocations.ambientLocation          = glGetUniformLocation(program, replaceLight(ii, "light%.ambient").c_str());
        lightLocations.diffuseLocation          = glGetUniformLocation(program, replaceLight(ii, "light%.diffuse").c_str());
        lightLocations.specularLocation         = glGetUniformLocation(program, replaceLight(ii, "light%.specular").c_str());
        lightLocations.diffuseScaleLocation     = glGetUniformLocation(program, replaceLight(ii, "light%.diffuseScale").c_str());
        lightLocations.diffuseTranslateLocation = glGetUniformLocation(program, replaceLight(ii, "light%.diffuseTranslate").c_str());

        lightLocations.constantLocation         = glGetUniformLocation(program, replaceLight(ii, "light%.constant").c_str());
        lightLocations.linearLocation           = glGetUniformLocation(program, replaceLight(ii, "light%.linear").c_str());
        lightLocations.quadraticLocation        = glGetUniformLocation(program, replaceLight(ii, "light%.quadratic").c_str());
        lightLocations.spotLightUVLocation      = glGetUniformLocation(program, replaceLight(ii, "light%.spotLightUV").c_str());
        lightLocations.spotLightWHLocation      = glGetUniformLocation(program, replaceLight(ii, "light%.spotLightWH").c_str());

        lightLocations.lightTextureIDLocation   = glGetUniformLocation(program, replaceLight(ii, "light%Texture").c_str());
        lightLocations.lightTextureSizeLocation = glGetUniformLocation(program, replaceLight(ii, "light%TextureSize").c_str());
      }
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

  Shader::use(shaderType);
}

//##################################################################################################
void MaterialShader::setMaterial(const Material& material)
{
  if(d->shaderType != ShaderType::Render)
    return;

  glUniform3fv(d->materialAmbientLocation,  1, &material.ambient.x  );
  glUniform3fv(d->materialDiffuseLocation,  1, &material.diffuse.x  );
  glUniform3fv(d->materialSpecularLocation, 1, &material.specular.x );
  glUniform1f(d->materialShininessLocation,     material.shininess  );
  glUniform1f(d->materialAlphaLocation,         material.alpha      );
}

//##################################################################################################
void MaterialShader::setLights(const std::vector<Light>& lights, const std::vector<FBO>& lightBuffers)
{
  if(d->shaderType != ShaderType::Render)
    return;

  {
    size_t iMax = tpMin(lights.size(), d->lightLocations.size());
    for(size_t i=0; i<iMax; i++)
    {
      const auto& light = lights.at(i);
      const auto& lightLocations = d->lightLocations.at(i);

      glm::vec3 position = light.position();
      glm::vec3 direction = light.direction();

      if(lightLocations.positionLocation>=0)
        glUniform3fv(lightLocations.positionLocation , 1,    &position.x            );
        
      glUniform3fv(lightLocations.directionLocation, 1,    &direction.x           );
      glUniform3fv(lightLocations.ambientLocation  , 1,    &light.ambient.x       );
      glUniform3fv(lightLocations.diffuseLocation  , 1,    &light.diffuse.x       );
      glUniform3fv(lightLocations.specularLocation , 1,    &light.specular.x      );
      glUniform1f (lightLocations.diffuseScaleLocation,     light.diffuseScale    );
      glUniform1f (lightLocations.diffuseTranslateLocation, light.diffuseTranslate);
      glUniform1f (lightLocations.constantLocation,         light.constant        );
      glUniform1f (lightLocations.linearLocation,           light.linear          );
      glUniform1f (lightLocations.quadraticLocation,        light.quadratic       );
      glUniform2fv(lightLocations.spotLightUVLocation, 1,  &light.spotLightUV.x   );
      glUniform2fv(lightLocations.spotLightWHLocation, 1,  &light.spotLightWH.x   );
    }
  }

  {
    size_t iMax = tpMin(lightBuffers.size(), d->lightLocations.size());
    for(size_t i=0; i<iMax; i++)
    {
      const auto& lightBuffer = lightBuffers.at(i);
      const auto& lightLocations = d->lightLocations.at(i);
      glUniformMatrix4fv(lightLocations.worldToLightLocation, 1, GL_FALSE, glm::value_ptr(lightBuffer.worldToTexture));

      glActiveTexture(GL_TEXTURE6 + i);
      glBindTexture(GL_TEXTURE_2D, lightBuffer.depthID);
      glUniform1i(lightLocations.lightTextureIDLocation, 6 + i);

      glm::vec2 lightTextureSize{lightBuffer.width, lightBuffer.height};
      glUniform2fv(lightLocations.lightTextureSizeLocation, 1, &lightTextureSize.x);
    }
  }
}

//##################################################################################################
void MaterialShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 mvp = p*v*m;

  switch(d->shaderType)
  {
  case ShaderType::Render:
  {
    glUniformMatrix4fv(d->mMatrixLocation  , 1, GL_FALSE, glm::value_ptr(m));
    glUniformMatrix4fv(d->mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));

    glm::vec3 cameraOrigin_world = glm::inverse(v) * glm::vec4(0,0,0,1);
    glUniform3fv(d->cameraOriginLocation, 1, &cameraOrigin_world.x);
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
void MaterialShader::drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::setTextures(GLuint ambientTextureID,
                                 GLuint diffuseTextureID,
                                 GLuint specularTextureID,
                                 GLuint alphaTextureID,
                                 GLuint bumpTextureID,
                                 GLuint spotLightTextureID)
{  
  if(d->shaderType != ShaderType::Render)
    return;

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ambientTextureID);
  glUniform1i(d->ambientTextureLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
  glUniform1i(d->diffuseTextureLocation, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, specularTextureID);
  glUniform1i(d->specularTextureLocation, 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, alphaTextureID);
  // glUniform1i(d->alphaTextureLocation, 3);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, bumpTextureID);
  glUniform1i(d->bumpTextureLocation, 4);

  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_2D, spotLightTextureID);
  glUniform1i(d->spotLightTextureLocation, 5);
}

}
