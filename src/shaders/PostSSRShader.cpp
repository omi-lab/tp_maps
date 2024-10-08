#include "tp_maps/shaders/PostSSRShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostSSRShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostSSRShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
