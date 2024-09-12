#include "tp_maps/shaders/PassThroughShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PassThroughShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PassThroughShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
