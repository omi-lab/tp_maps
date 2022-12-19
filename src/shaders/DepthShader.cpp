#include "tp_maps/shaders/DepthShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/DepthShader.frag"}; return s;}
}

//##################################################################################################
DepthShader::DepthShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::Render))
{

}

}
