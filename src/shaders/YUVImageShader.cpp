#include "tp_maps/shaders/YUVImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource fragShaderStr{"/tp_maps/YUVImageShader.frag"};
}

//##################################################################################################
YUVImageShader::YUVImageShader(tp_maps::OpenGLProfile openGLProfile):
  ImageShader(openGLProfile, nullptr, fragShaderStr.data(openGLProfile))
{

}

}
