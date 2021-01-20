#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostSSAOShader.frag"}; return s;}

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
struct PostSSAOShaderPrivate::Private
{
  PostSSAOParameters parameters;

  std::string fragSrc;
  size_t maxLights{0};

  std::vector<glm::vec3> ssaoKernel;

  GLint ssaoKernelLocation{0};
  std::vector<LightLocations_lt> lightLocations;


  //################################################################################################
  Private(Map* map, tp_maps::OpenGLProfile openGLProfile, const PostSSAOParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderHDR);

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
          maxLights = size_t(textureUnits) - size_t(staticTextures);
      }

      const auto& lights = map->lights();
      size_t iMax = tpMin(maxLights, lights.size());
      for(size_t i=0; i<iMax; i++)
      {
        const auto& light = lights.at(i);
        auto ii = std::to_string(i);

        size_t levels = (light.type==tp_math_utils::LightType::Spot)?map->spotLightLevels():1;
        auto ll = std::to_string(levels);

        AO_FRAG_VARS += replaceLight(ii, ll, "uniform mat4 viewToLight%;\n");
        AO_FRAG_VARS += replaceLight(ii, ll, "uniform mat4 invViewToLight%;\n");

        if(map->spotLightLevels() == 1)
        {
          AO_FRAG_VARS += replaceLight(ii, ll, "uniform sampler2D light%Texture;\n");
          AO_FRAG_CALC += replaceLight(ii, ll, "    occlusion = min(testBuffer2D(coord_view, samplePos_view, viewToLight%, invViewToLight%, light%Texture), occlusion);\n");
        }
        else
        {
          AO_FRAG_VARS += replaceLight(ii, ll, "uniform sampler3D light%Texture;\n");
          AO_FRAG_CALC += replaceLight(ii, ll, "    occlusion = min(testBuffer3D(coord_view, samplePos_view, viewToLight%, invViewToLight%, light%Texture), occlusion);\n");
        }
      }
    }

    replace(fragSrc, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);
    replace(fragSrc, "/*AO_FRAG_CALC*/", AO_FRAG_CALC);
  }
};

//##################################################################################################
PostSSAOShaderPrivate::PostSSAOShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const PostSSAOParameters& parameters):
d(new Private(map, openGLProfile, parameters))
{

}

//##################################################################################################
PostSSAOShader::PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const PostSSAOParameters& parameters):
  PostSSAOShaderPrivate(map, openGLProfile, parameters),
  PostShader(map, openGLProfile, PostSSAOShaderPrivate::d->fragSrc),
  d(PostSSAOShaderPrivate::d)
{
  {
    auto s = shaderDetails(ShaderType::RenderHDR);
    d->ssaoKernelLocation = glGetUniformLocation(s.program, "ssaoKernel");

    const auto& lights = map->lights();
    size_t iMax = tpMin(d->maxLights, lights.size());

    d->lightLocations.resize(iMax);
    for(size_t i=0; i<d->lightLocations.size(); i++)
    {
      auto& lightLocations = d->lightLocations.at(i);

      auto ii = std::to_string(i);

      lightLocations.   viewToLightProjLocation = glGetUniformLocation(s.program, replaceLight(ii, "",    "viewToLight%").c_str());
      lightLocations.invViewToLightProjLocation = glGetUniformLocation(s.program, replaceLight(ii, "", "invViewToLight%").c_str());
      lightLocations.lightTextureIDLocation     = glGetUniformLocation(s.program, replaceLight(ii, "", "light%Texture"  ).c_str());
    }
  }

  std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
  std::default_random_engine generator;

  for(size_t i=0; i<parameters.nSamples; i++)
  {
    glm::vec3 sample(
          randomFloats(generator) * 2.0f - 1.0f,
          randomFloats(generator) * 2.0f - 1.0f,
          randomFloats(generator)
          );
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
void PostSSAOShader::use(ShaderType shaderType)
{
  PostShader::use(shaderType);
  glUniform3fv(d->ssaoKernelLocation, GLsizei(d->parameters.nSamples), &d->ssaoKernel[0][0]);

  if(d->parameters.useLightBuffers)
    setLights(map()->lights(), map()->lightBuffers());
}


//##################################################################################################
void PostSSAOShader::setLights(const std::vector<tp_math_utils::Light>& lights, const std::vector<FBO>& lightBuffers)
{
  TP_UNUSED(lights);

  {
    size_t iMax = tpMin(lightBuffers.size(), d->lightLocations.size());
    for(size_t i=0; i<iMax; i++)
    {
      const auto& lightBuffer = lightBuffers.at(i);
      const auto& lightLocations = d->lightLocations.at(i);

      auto invCameraV = glm::inverse(map()->controller()->matrices(tp_maps::defaultSID()).v);
      const auto& lightV = lightBuffer.worldToTexture[0].v;
      const auto& lightP = lightBuffer.worldToTexture[0].p;

      glm::mat4 viewToLight = lightP * lightV * invCameraV;
      glm::mat4 invViewToLight = glm::inverse(viewToLight);

      glUniformMatrix4fv(lightLocations.   viewToLightProjLocation, 1, GL_FALSE, glm::value_ptr(  viewToLight));
      glUniformMatrix4fv(lightLocations.invViewToLightProjLocation, 1, GL_FALSE, glm::value_ptr(invViewToLight));

      glActiveTexture(GLenum(GL_TEXTURE6 + i));

      if(lightBuffer.levels == 1)
        glBindTexture(GL_TEXTURE_2D, lightBuffer.depthID);
      else
        glBindTexture(GL_TEXTURE_3D, lightBuffer.depthID);

      glUniform1i(lightLocations.lightTextureIDLocation, GLint(6 + i));
    }
  }
}
}
