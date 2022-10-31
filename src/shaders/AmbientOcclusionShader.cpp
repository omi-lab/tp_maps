#include "tp_maps/shaders/AmbientOcclusionShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/AmbientOcclusionShader.frag"}; return s;}
}

namespace detail
{
//##################################################################################################
struct AmbientOcclusionShaderPrivate::Private
{
  AmbientOcclusionParameters parameters;

  std::string fragSrc;
  size_t maxLights{0};

  std::vector<glm::vec3> ssaoKernel;

  GLint ssaoKernelLocation{0};

  GLuint ssaoNoiseTexture{0};
  GLuint ssaoNoiseTextureLocation{0};


  //################################################################################################
  Private(tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderExtendedFBO);

    std::string AO_FRAG_VARS;
    std::string AO_FRAG_CALC;

    AO_FRAG_VARS += "const float radius=" + std::to_string(parameters.radius) + ";\n";
    AO_FRAG_VARS += "#define N_SAMPLES " + std::to_string(parameters.nSamples) + "\n";
    AO_FRAG_VARS += "const float bias=" + std::to_string(parameters.bias) + ";\n";

    if(parameters.useScreenBuffer)
    {
      AO_FRAG_CALC += "    occlusion = min(testBuffer2D(coord_view, samplePos_view, projectionMatrix, invProjectionMatrix, depthSampler), occlusion);\n";
    }

    tp_utils::replace(fragSrc, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);
    tp_utils::replace(fragSrc, "/*AO_FRAG_CALC*/", AO_FRAG_CALC);
  }

  //################################################################################################
  std::function<void(GLuint)> bindLocations(const std::function<void(GLuint)>& bindLocations = std::function<void(GLuint)>())
  {
    return bindLocations;
  }

  //################################################################################################
  std::function<void(GLuint)> getLocations(const std::function<void(GLuint)>& getLocations = std::function<void(GLuint)>())
  {
    return [=](GLuint program)
    {
      ssaoNoiseTextureLocation          = glGetUniformLocation(program, "noiseSampler");

      if(getLocations)
        getLocations(program);
    };
  }
};

//##################################################################################################
AmbientOcclusionShaderPrivate::AmbientOcclusionShaderPrivate(Map*, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters):
  d(new Private(openGLProfile, parameters))
{

}
}

//##################################################################################################
AmbientOcclusionShader::AmbientOcclusionShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters):
  AmbientOcclusionShaderPrivate(map, openGLProfile, parameters),
  PostShader(map, openGLProfile, nullptr, d->fragSrc.data(), d->bindLocations(), d->getLocations() )
{
  {
    auto s = shaderDetails(ShaderType::RenderExtendedFBO);
    d->ssaoKernelLocation = glGetUniformLocation(s.program, "ssaoKernel");
  }

  std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
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

    float scale = float(i) / 64.0f;

    auto lerp = [](float a, float b, float f){
      return a + f * (b - a);
    };

    // scale samples s.t. they're more aligned to center of kernel
    scale = lerp(0.1f, 1.0f, scale * scale);
    sample *= scale;

    d->ssaoKernel.push_back(sample);
  }

  // Generate noise texture
  std::vector<glm::vec3> ssaoNoise;
  for (unsigned int i = 0; i < 16; i++)
  {
      glm::vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, 0.0f); // rotate around z-axis (in tangent space)
      ssaoNoise.push_back(noise);
  }

  glGenTextures(1, &d->ssaoNoiseTexture);
  glBindTexture(GL_TEXTURE_2D, d->ssaoNoiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

//##################################################################################################
AmbientOcclusionShader::~AmbientOcclusionShader()
{
  delete d;
}

//##################################################################################################
void AmbientOcclusionShader::compile(const char* vertexShader,
                                     const char* fragmentShader,
                                     const std::function<void(GLuint)>& bindLocations,
                                     const std::function<void(GLuint)>& getLocations,
                                     ShaderType shaderType)
{
  FullScreenShader::compile(vertexShader,
                            fragmentShader,
                            d->bindLocations(bindLocations),
                            d->getLocations(getLocations),
                            shaderType);
}


//##################################################################################################
void AmbientOcclusionShader::use(ShaderType shaderType)
{
  PostShader::use(shaderType);
  glUniform3fv(d->ssaoKernelLocation, GLsizei(d->parameters.nSamples), &d->ssaoKernel[0][0]);

  // Bind noise texture
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, d->ssaoNoiseTexture);
  glUniform1i(d->ssaoNoiseTextureLocation, 4);
}

}
