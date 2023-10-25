#include "tp_maps/shaders/G3DYUVImageShader.h"

namespace tp_maps
{

//##################################################################################################
const char* G3DYUVImageShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/YUVImageShader.frag"};
  return s.data(openGLProfile(), shaderType);
}

}
