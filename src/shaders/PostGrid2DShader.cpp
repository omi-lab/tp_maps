#include "tp_maps/shaders/PostGrid2DShader.h"

namespace tp_maps
{
//##################################################################################################
const std::string& PostGrid2DShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostGrid2DShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}
}
