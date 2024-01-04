#include "tp_maps/shaders/PostViewTransformShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostViewTransformShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostViewTransformShader.frag"};
  return s.data(shaderProfile(), shaderType);
}
}
