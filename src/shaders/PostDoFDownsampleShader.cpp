#include "tp_maps/shaders/PostDoFDownsampleShader.h"

namespace tp_maps
{

//##################################################################################################
const std::string& PostDoFDownsampleShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/DownsampleShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

}
