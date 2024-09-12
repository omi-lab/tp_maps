#include "tp_maps/shaders/PostBlurAndTintShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostBlurAndTintShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostBlurAndTintShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
