#include "tp_maps/shaders/PostGammaShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostGammaShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostGammaShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
