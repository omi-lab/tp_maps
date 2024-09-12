#include "tp_maps/shaders/G3DYUVImageShader.h"

namespace tp_maps
{

//##################################################################################################
const std::string& G3DYUVImageShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/YUVImageShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

}
