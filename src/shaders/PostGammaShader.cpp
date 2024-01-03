#include "tp_maps/shaders/PostGammaShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostGammaShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostGammaShader.frag"};
  return s.data(shaderProfile(), shaderType);
}
}
