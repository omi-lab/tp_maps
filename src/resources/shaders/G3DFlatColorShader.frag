#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

uniform vec4 color;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  TP_GLSL_GLFRAGCOLOR = color;
  if(TP_GLSL_GLFRAGCOLOR.a < 0.01)
    discard;
}
