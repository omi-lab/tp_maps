#include "tp_maps/shaders/PostBlurAndTintShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostBlurAndTintShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostBlurAndTintShader.frag"};
  return s.data(openGLProfile(), shaderType);
}
}
