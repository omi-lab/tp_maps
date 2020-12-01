#include "tp_maps/shaders/PostSSAOShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostSSAOShader.frag"}; return s;}
}

//##################################################################################################
PostSSAOShader::PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::Render))
{

}

}
