#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
struct PostOutlineShader::Private
{
  GLint depthObjectLocation{-1};
};

//##################################################################################################
PostOutlineShader::PostOutlineShader(tp_maps::Map* map, tp_maps::ShaderProfile shaderProfile):
  PostShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
PostOutlineShader::~PostOutlineShader()
{
  delete d;
}

//##################################################################################################
void PostOutlineShader::use(tp_maps::ShaderType shaderType)
{
  tp_maps::PostShader::use(shaderType);
  //  glUniform1f(d->pulseOpacityLocation, pulseOpacity);
  //  glUniform1f(d->pulseOffsetLocation, pulseOffset);
  //  glUniform2fv(d->pulseOriginLocation, 1, glm::value_ptr(pulseOrigin));

  //  const tp_maps::OpenGLFBO* fbo = nullptr;

  // Object Depth map
  if(auto fbo = map()->intermediateBuffer(tp_maps::selectionPassSID()); fbo && d->depthObjectLocation >= 0)
  {
    static constexpr int32_t bindingPoint = 4;
    glActiveTexture(GL_TEXTURE0 + bindingPoint);
    glBindTexture(GL_TEXTURE_2D, fbo->depthID);
    glUniform1i(d->depthObjectLocation, bindingPoint);
  }

  //  // Scene Depth map
  //  fbo = map()->intermediateBuffer(tp_maps::defaultSID());
  //  if (fbo && d->depthSceneLocation >= 0)
  //  {
  //    static constexpr int32_t bindingPoint = 5;
  //    glActiveTexture(GL_TEXTURE0 + bindingPoint);
  //    glBindTexture(GL_TEXTURE_2D, fbo->depthID);
  //    glUniform1i(d->depthSceneLocation, bindingPoint);
  //  }
}

//##################################################################################################
const std::string& PostOutlineShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostOutlineShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

//##################################################################################################
void PostOutlineShader::getLocations(GLuint program, tp_maps::ShaderType shaderType)
{
  tp_maps::PostShader::getLocations(program, shaderType);
  d->depthObjectLocation  = glGetUniformLocation(program, "depthObjectSampler");
}
}
