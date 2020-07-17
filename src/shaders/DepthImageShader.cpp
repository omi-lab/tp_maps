#include "tp_maps/shaders/DepthImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource fragShaderStr{"/tp_maps/DepthImageShader.frag"};
}

//##################################################################################################
DepthImageShader::DepthImageShader(tp_maps::OpenGLProfile openGLProfile):
  ImageShader(openGLProfile, nullptr, fragShaderStr.data(openGLProfile))
{

}

}
