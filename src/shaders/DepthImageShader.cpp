#include "tp_maps/shaders/DepthImageShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/DepthImageShader.frag"}; return s;}
ShaderResource& frag3DShaderStr(){static ShaderResource s{"/tp_maps/DepthImage3DShader.frag"}; return s;}
}

//##################################################################################################
DepthImageShader::DepthImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::Render))
{

}

//##################################################################################################
DepthImage3DShader::DepthImage3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, frag3DShaderStr().data(openGLProfile, ShaderType::Render))
{

}

}
