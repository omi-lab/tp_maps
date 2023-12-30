#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec3 LightVector0;
TP_GLSL_IN_F vec3 EyeNormal;
TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform vec4 color;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

mat4 ycbcrToRGBTransform = mat4(
vec4(+1.0000f, +1.0000f, +1.0000f, +0.0000f),
vec4(+0.0000f, -0.3441f, +1.7720f, +0.0000f),
vec4(+1.4020f, -0.7141f, +0.0000f, +0.0000f),
vec4(-0.7010f, +0.5291f, -0.8860f, +1.0000f)
);

void main()
{
  TP_GLSL_GLFRAGCOLOR = (ycbcrToRGBTransform * TP_GLSL_TEXTURE_2D(textureSampler, coord_tex))*color;
  if(TP_GLSL_GLFRAGCOLOR.a < 0.01)
    discard;
}
