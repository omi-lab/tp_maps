#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;

#pragma replace DOF_FRAG_VARS
#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

const vec2 offsets[4] = vec2[]
(
  vec2(-0.5, -0.5),
  vec2(-0.5, 0.5),
  vec2(0.5, -0.5),
  vec2(0.5,0.5)
);

void main()
{
  vec4 color = vec4(0.0,0.0,0.0,0.0);

  color += TP_GLSL_TEXTURE_2D(textureSampler, coord_tex + offsets[0] * pixelSize);
  color += TP_GLSL_TEXTURE_2D(textureSampler, coord_tex + offsets[1] * pixelSize);
  color += TP_GLSL_TEXTURE_2D(textureSampler, coord_tex + offsets[2] * pixelSize);
  color += TP_GLSL_TEXTURE_2D(textureSampler, coord_tex + offsets[3] * pixelSize);

  TP_GLSL_GLFRAGCOLOR = color / 4.0;
}
