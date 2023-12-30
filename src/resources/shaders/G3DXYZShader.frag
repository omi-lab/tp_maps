#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec3 vertex;
TP_GLSL_IN_F vec2 texCoord;

uniform sampler2D textureSampler;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

//##################################################################################################
void main()
{
  TP_GLSL_GLFRAGCOLOR = TP_GLSL_TEXTURE_2D(textureSampler, texCoord);
  if(TP_GLSL_GLFRAGCOLOR.a < 0.01)
    discard;

  TP_GLSL_GLFRAGCOLOR = vec4(vertex, 1.0);
}
