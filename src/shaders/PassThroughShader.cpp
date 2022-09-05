#include "tp_maps/shaders/PassThroughShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PassThroughShader.frag"}; return s;}
}

//##################################################################################################
PassThroughShader::PassThroughShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
