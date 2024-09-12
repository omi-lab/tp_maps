#include "tp_maps/shaders/G3DPickingShader.h"

namespace tp_maps
{

//##################################################################################################
const std::string& G3DPickingShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DPickingShader.frag"};

  if(shaderType == ShaderType::Picking)
    return G3DImageShader::fragmentShaderStr(shaderType);

  return s.dataStr(shaderProfile(), shaderType);
}

}
