#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR

TP_GLSL_IN_F vec2 inTexture;

TP_GLSL_IN_F vec3 LightVector0;

uniform sampler2D textureSampler;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  float depth = gl_FragCoord.z;
  TP_GLSL_GLFRAGCOLOR = vec4(depth, depth, depth, 1.0);
}
