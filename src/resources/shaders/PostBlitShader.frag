#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  TP_GLSL_GLFRAGCOLOR = vec4(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).xyz, 1.0);
}
