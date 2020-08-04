#include "tp_maps/shaders/DepthImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource fragShaderStr{"/tp_maps/DepthImageShader.frag"};
}

//##################################################################################################
DepthImageShader::DepthImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, fragShaderStr.data(openGLProfile))
{

}

}
