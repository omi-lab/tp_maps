#include "tp_maps/shaders/PostOutlineShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostOutlineShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostOutlineShader.frag"};
  return s.data(shaderProfile(), shaderType);
}
}
