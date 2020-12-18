#include "tp_maps/shaders/PostBlitShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostBlitShader.frag"}; return s;}
}

//##################################################################################################
PostBlitShader::PostBlitShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderHDR))
{

}

}
