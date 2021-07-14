#include "tp_maps/shaders/PostGammaShader.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostGammaShader.frag"}; return s;}
}

//##################################################################################################
PostGammaShader::PostGammaShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
