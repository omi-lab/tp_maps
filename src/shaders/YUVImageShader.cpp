#include "tp_maps/shaders/YUVImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/YUVImageShader.frag"}; return s;}
}

//##################################################################################################
YUVImageShader::YUVImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile))
{

}

}
