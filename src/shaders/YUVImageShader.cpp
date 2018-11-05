#include "tp_maps/shaders/YUVImageShader.h"

namespace tp_maps
{

namespace
{

ShaderString fragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//YUVImageShader fragmentShaderStr\n"
    "$TP_GLSL_IN_F$vec3 LightVector0;\n"
    "$TP_GLSL_IN_F$vec3 EyeNormal;\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "uniform sampler2D textureSampler;\n"
    "uniform vec4 color;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "mat4 ycbcrToRGBTransform = mat4("
    "vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),"
    "vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),"
    "vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),"
    "vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)"
    ");\n"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = (ycbcrToRGBTransform * $TP_GLSL_TEXTURE$(textureSampler, texCoordinate))*color;\n"
    "  if($TP_GLSL_GLFRAGCOLOR$.a < 0.01)\n"
    "    discard;\n"
    "}\n";
}

//##################################################################################################
YUVImageShader::YUVImageShader():
  ImageShader(nullptr, fragmentShaderStr.data())
{

}

}
