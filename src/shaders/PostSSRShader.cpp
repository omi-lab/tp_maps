#include "tp_maps/shaders/PostSSRShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostSSRShader.frag"}; return s;}
}

//##################################################################################################
PostSSRShader::PostSSRShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
