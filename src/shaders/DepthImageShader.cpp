#include "tp_maps/shaders/DepthImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/DepthImageShader.frag"}; return s;}
}

//##################################################################################################
DepthImageShader::DepthImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile))
{

}

}
