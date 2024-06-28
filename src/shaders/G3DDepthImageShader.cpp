#include "tp_maps/shaders/G3DDepthImageShader.h"

namespace tp_maps
{

//##################################################################################################
const char* G3DDepthImageShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DDepthImageShader.frag"};

  if(shaderType == ShaderType::Picking)
    return G3DImageShader::fragmentShaderStr(shaderType);

  return s.data(shaderProfile(), shaderType);
}

}
