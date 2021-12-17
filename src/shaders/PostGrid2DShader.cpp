#include "tp_maps/shaders/PostGrid2DShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostGrid2DShader.frag"}; return s;}
}

//##################################################################################################
PostGrid2DShader::PostGrid2DShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
