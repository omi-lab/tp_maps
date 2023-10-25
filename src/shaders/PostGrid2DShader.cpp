#include "tp_maps/shaders/PostGrid2DShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostGrid2DShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostGrid2DShader.frag"};
  return s.data(openGLProfile(), shaderType);
}
}
