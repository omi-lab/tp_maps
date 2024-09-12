#include "tp_maps/shaders/G3DDepthShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& G3DDepthShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DDepthShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
