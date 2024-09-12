#include "tp_maps/shaders/PostOutlineShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostOutlineShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostOutlineShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
