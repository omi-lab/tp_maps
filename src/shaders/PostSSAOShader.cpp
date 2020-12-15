#include "tp_maps/shaders/PostSSAOShader.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostSSAOShader.frag"}; return s;}
}

//##################################################################################################
struct PostSSAOShader::Private
{
  GLint ssaoKernelLocation{0};

  std::vector<glm::vec3> ssaoKernel;
};

//##################################################################################################
PostSSAOShader::PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::Render)),
  d(new Private())
{
  {
    auto s = shaderDetails(ShaderType::Render);
    d->ssaoKernelLocation = glGetUniformLocation(s.program, "ssaoKernel");
  }

  std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
  std::default_random_engine generator;

  for(int i = 0; i<64; i++)
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
  glUniform3fv(d->ssaoKernelLocation, 64, &d->ssaoKernel[0][0]);
}
}
