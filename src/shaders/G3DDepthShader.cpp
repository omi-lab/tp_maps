#include "tp_maps/shaders/G3DDepthShader.h"

namespace tp_maps
{
//##################################################################################################
const char* G3DDepthShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/DepthShader.frag"};
  return s.data(openGLProfile(), shaderType);
}
}
