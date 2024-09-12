#include "tp_maps/shaders/PostBlitShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostBlitShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostBlitShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
