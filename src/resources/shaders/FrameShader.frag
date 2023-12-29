#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec3 lightVector0;
TP_GLSL_IN_F vec4 eyeTBNq;
TP_GLSL_IN_F vec2 coord_tex;
TP_GLSL_IN_F vec4 multColor;

uniform sampler2D textureSampler;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  TP_GLSL_GLFRAGCOLOR = TP_GLSL_TEXTURE_2D(textureSampler, coord_tex)*multColor;
  if(TP_GLSL_GLFRAGCOLOR.a < 0.01)
    discard;
}
