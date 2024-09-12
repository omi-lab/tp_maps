#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;

const float discardOpacity=0.8;

uniform mat4 vpInv;

uniform float rotationFactor;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF
#pragma replace TP_WRITE_FRAGMENT
#pragma replace TP_COLOR_MANAGEMENT

//##################################################################################################
void main()
{
  vec4 v = vec4(coord_tex*2.0 - 1.0, 0.0, 1.0);

  v.z=0.0;
  vec4 near = vpInv * v;

  v.z=1.0;
  vec4 far = vpInv * v;

  vec3 d = normalize((far.xyz / far.w) - (near.xyz / near.w));

  vec2 textureCoord;
  textureCoord.x = 1.0 - (atan(d.y, d.x) / (2.0*3.1415926538) + 0.5);
  textureCoord.y = acos(d.z) / 3.1415926538;

  textureCoord.x += rotationFactor;

  //Note: GammaCorrection
  vec3 ambient = toLinear(TP_GLSL_TEXTURE_2D(textureSampler, textureCoord).xyz);

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  float alpha = 1.0;

  writeFragment(ambient, diffuse, specular, alpha);
}
