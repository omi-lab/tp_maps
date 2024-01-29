#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
//##################################################################################################
struct LightLocations_lt
{
  // The matrix that transforms view coords onto the light texture.
  GLint viewToLightProjLocation{0};
  GLint invViewToLightProjLocation{0};

  GLint   lightTextureIDLocation{0};
};
}

//##################################################################################################
struct PostSSAOShader::Private
{
  PostSSAOParameters parameters;

  size_t maxLights{0};

  std::vector<glm::vec3> ssaoKernel;

  GLint ssaoKernelLocation{0};
  std::vector<LightLocations_lt> lightLocations;


  //################################################################################################
  Private(const PostSSAOParameters& parameters_):
    parameters(parameters_)
  {
  }
};

//##################################################################################################
PostSSAOShader::PostSSAOShader(Map* map, tp_maps::ShaderProfile shaderProfile, const PostSSAOParameters& parameters):
  PostShader(map, shaderProfile),
  d(new Private(parameters))
{
  std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
  std::default_random_engine generator;

  for(size_t i=0; i<parameters.nSamples; i++)
  {
    glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f,
                     randomFloats(generator) * 2.0f - 1.0f,
                     randomFloats(generator));
    sample  = glm::normalize(sample);
    sample *= randomFloats(generator);
    d->ssaoKernel.push_back(sample);
  }
}

//##################################################################################################
PostSSAOShader::~PostSSAOShader()
{
  delete d;
}

//##################################################################################################
void PostSSAOShader::setLights(const std::vector<tp_math_utils::Light>& lights, const std::vector<OpenGLFBO>& lightBuffers)
{
  TP_UNUSED(lights);

  {
    size_t iMax = tpMin(lightBuffers.size(), d->lightLocations.size());
    for(size_t i=0; i<iMax; i++)
    {
      const auto& lightBuffer = lightBuffers.at(i);
      const auto& lightLocations = d->lightLocations.at(i);

      auto invCameraV = glm::inverse(map()->controller()->matrices(tp_maps::defaultSID()).v);
      const auto& lightV = lightBuffer.worldToTexture.v;
      const auto& lightP = lightBuffer.worldToTexture.p;

      glm::mat4 viewToLight = lightP * lightV * invCameraV;
      glm::mat4 invViewToLight = glm::inverse(viewToLight);

      glUniformMatrix4fv(lightLocations.   viewToLightProjLocation, 1, GL_FALSE, glm::value_ptr(  viewToLight));
      glUniformMatrix4fv(lightLocations.invViewToLightProjLocation, 1, GL_FALSE, glm::value_ptr(invViewToLight));

      glActiveTexture(GLenum(GL_TEXTURE6 + i));
      glBindTexture(GL_TEXTURE_2D, lightBuffer.depthID);

      glUniform1i(lightLocations.lightTextureIDLocation, GLint(6 + i));
    }
  }
}

//##################################################################################################
void PostSSAOShader::use(ShaderType shaderType)
{
  PostShader::use(shaderType);
  glUniform3fv(d->ssaoKernelLocation, GLsizei(d->parameters.nSamples), &d->ssaoKernel[0][0]);

  if(d->parameters.useLightBuffers)
    setLights(map()->lights(), map()->lightBuffers());
}

//##################################################################################################
const char* PostSSAOShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostSSAOShader.frag"};
  fragSrcScratch = s.dataStr(shaderProfile(), shaderType);

  const auto& parameters = d->parameters;
  auto map = this->map();

  std::string AO_FRAG_VARS;
  std::string AO_FRAG_CALC;

  AO_FRAG_VARS += "const float radius=" + std::to_string(parameters.radius) + ";\n";
  AO_FRAG_VARS += "#define N_SAMPLES " + std::to_string(parameters.nSamples) + "\n";

  if(parameters.useScreenBuffer)
  {
    AO_FRAG_CALC += "    occlusion = min(testBuffer2D(coord_view, samplePos_view, projectionMatrix, invProjectionMatrix, depthSampler), occlusion);\n";
  }

  if(parameters.useLightBuffers)
  {
    {
      //The number of lights we can used is limited by the number of available texture units.
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
      auto ii = std::to_string(i);

      AO_FRAG_VARS += replaceLight(ii, "uniform mat4 viewToLight%;\n");
      AO_FRAG_VARS += replaceLight(ii, "uniform mat4 invViewToLight%;\n");

      AO_FRAG_VARS += replaceLight(ii, "uniform sampler2D light%Texture;\n");
      AO_FRAG_CALC += replaceLight(ii, "    occlusion = min(testBuffer2D(coord_view, samplePos_view, viewToLight%, invViewToLight%, light%Texture), occlusion);\n");
    }
  }

  tp_utils::replace(fragSrcScratch, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);
  tp_utils::replace(fragSrcScratch, "/*AO_FRAG_CALC*/", AO_FRAG_CALC);

  return fragSrcScratch.c_str();
}

//##################################################################################################
void PostSSAOShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  d->ssaoKernelLocation = glGetUniformLocation(program, "ssaoKernel");

  const auto& lights = map()->lights();
  size_t iMax = tpMin(d->maxLights, lights.size());

  d->lightLocations.resize(iMax);
  for(size_t i=0; i<d->lightLocations.size(); i++)
  {
    auto& lightLocations = d->lightLocations.at(i);

    auto ii = std::to_string(i);

    lightLocations.   viewToLightProjLocation = glGetUniformLocation(program, replaceLight(ii,    "viewToLight%").c_str());
    lightLocations.invViewToLightProjLocation = glGetUniformLocation(program, replaceLight(ii, "invViewToLight%").c_str());
    lightLocations.lightTextureIDLocation     = glGetUniformLocation(program, replaceLight(ii, "light%Texture"  ).c_str());
  }
}

}
