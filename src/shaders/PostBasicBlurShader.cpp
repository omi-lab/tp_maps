#include "tp_maps/shaders/PostBasicBlurShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostBasicBlurShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostBasicBlurShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
