#include "tp_maps/shaders/PostDoFDownsampleShader.h"

namespace tp_maps
{

//##################################################################################################
const char* PostDoFDownsampleShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/DownsampleShader.frag"};
  return s.data(openGLProfile(), shaderType);
}

}
