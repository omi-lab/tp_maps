#include "tp_maps/shaders/PostAOShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
struct PostAOShader::Private
{
  Q* q;

  size_t maxLights{0};

  std::vector<glm::vec3> ssaoKernel;

  GLuint ssaoNoiseTexture{0};
  GLint ssaoKernelLocation{0};
  GLuint ssaoNoiseTextureLocation{0};

  //################################################################################################
  ~Private()
  {
    if(q->map() && ssaoNoiseTexture)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(ssaoNoiseTexture);
    }
  }
};

//##################################################################################################
PostAOShader::PostAOShader(Map* map,
                           tp_maps::ShaderProfile shaderProfile,
                           const PostAOParameters& parameters):
  PostAOBaseShader(map, shaderProfile, parameters),
  d(new Private())
{

  std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
  std::default_random_engine generator;

  for(size_t i=0; i<parameters.nSamples; i++)
  {
    glm::vec3 sample(randomFloats(generator) * 2.0f - 1.0f,
                     randomFloats(generator) * 2.0f - 1.0f,
                     randomFloats(generator));
    sample  = glm::normalize(sample);
    sample *= randomFloats(generator);

    float scale = float(i) / parameters.nSamples;
    auto lerp = [](float a, float b, float f){return a + f * (b - a);};

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

  auto colorFormatF = [map](Alpha alpha)
  {
#ifdef TP_GLES2
    return (alpha==Alpha::Yes)?GL_RGBA32F_EXT:GL_RGB16F_EXT;
#else
    switch(map->shaderProfile())
    {
        case ShaderProfile::GLSL_100_ES: [[fallthrough]];
        case ShaderProfile::GLSL_300_ES: [[fallthrough]];
        case ShaderProfile::GLSL_310_ES: [[fallthrough]];
        case ShaderProfile::GLSL_320_ES:
    return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB16F;

    default:
    return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB32F;
    }
#endif
  };

  glGenTextures(1, &d->ssaoNoiseTexture);
  glBindTexture(GL_TEXTURE_2D, d->ssaoNoiseTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(Alpha::No), 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

//##################################################################################################
PostAOShader::~PostAOShader()
{
  delete d;
}

//##################################################################################################
void PostAOShader::use(ShaderType shaderType)
{
  PostShader::use(shaderType);
  glUniform3fv(d->ssaoKernelLocation, GLsizei(parameters().nSamples), &d->ssaoKernel[0][0]);

  // Bind noise texture
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, d->ssaoNoiseTexture);
  glUniform1i(d->ssaoNoiseTextureLocation, 4);
}

//##################################################################################################
const char* PostAOShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/AmbientOcclusionShader.frag"};
  fragSrcScratch = s.dataStr(shaderProfile(), shaderType);

  const auto& parameters = this->parameters();

  std::string AO_FRAG_VARS;
  std::string AO_FRAG_CALC;

  AO_FRAG_VARS += "const float radius=" + std::to_string(parameters.radius) + ";\n";
  AO_FRAG_VARS += "#define N_SAMPLES " + std::to_string(parameters.nSamples > 0 ? parameters.nSamples : 1 ) + "\n";
  AO_FRAG_VARS += "const float bias=" + std::to_string(parameters.bias) + ";\n";

  if(parameters.useScreenBuffer)
  {
    AO_FRAG_CALC += "    occlusion = min(testBuffer2D(coord_view, samplePos_view, projectionMatrix, invProjectionMatrix, depthSampler), occlusion);\n";
  }

  tp_utils::replace(fragSrcScratch, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);
  tp_utils::replace(fragSrcScratch, "/*AO_FRAG_CALC*/", AO_FRAG_CALC);

  return fragSrcScratch.c_str();
}

//##################################################################################################
void PostAOShader::getLocations(GLuint program, ShaderType shaderType)
{
  PostAOBaseShader::getLocations(program, shaderType);
  d->ssaoKernelLocation       = glGetUniformLocation(program, "ssaoKernel");
  d->ssaoNoiseTextureLocation = glGetUniformLocation(program, "noiseSampler");
}

//##################################################################################################
void PostAOShader::invalidate()
{
  d->ssaoNoiseTexture = 0;
  PostShader::invalidate();
}

}
