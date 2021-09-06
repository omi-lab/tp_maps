#include "tp_maps/shaders/PostBlurAndTintShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostBlurAndTintShader.frag"}; return s;}
}

//##################################################################################################
PostBlurAndTintShader::PostBlurAndTintShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
