#include "tp_maps/shaders/PostBasicBlurShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PostBasicBlurShader.frag"}; return s;}
}

//##################################################################################################
PostBasicBlurShader::PostBasicBlurShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  PostShader(map, openGLProfile, nullptr, fragShaderStr().data(openGLProfile, ShaderType::RenderExtendedFBO))
{

}

}
