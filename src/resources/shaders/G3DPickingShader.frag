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

int getBit(int value, int srcIdx, int dstIdx)
{
  return ((value >> srcIdx) & 1) << dstIdx;
}

void main()
{
  int ro = int(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).x * 255.0);
  int go = int(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).y * 255.0);
  int bo = int(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).z * 255.0);

  int value = ro | (go<<8) | (bo<<16);

  int rs=0;
  int gs=0;
  int bs=0;

  rs |= getBit(value, 21, 0);
  rs |= getBit(value, 18, 1);
  rs |= getBit(value, 15, 2);
  rs |= getBit(value, 12, 3);
  rs |= getBit(value,  9, 4);
  rs |= getBit(value,  6, 5);
  rs |= getBit(value,  3, 6);
  rs |= getBit(value,  0, 7);

  gs |= getBit(value, 22, 0);
  gs |= getBit(value, 19, 1);
  gs |= getBit(value, 16, 2);
  gs |= getBit(value, 13, 3);
  gs |= getBit(value, 10, 4);
  gs |= getBit(value,  7, 5);
  gs |= getBit(value,  4, 6);
  gs |= getBit(value,  1, 7);

  bs |= getBit(value, 23, 0);
  bs |= getBit(value, 20, 1);
  bs |= getBit(value, 17, 2);
  bs |= getBit(value, 14, 3);
  bs |= getBit(value, 11, 4);
  bs |= getBit(value,  8, 5);
  bs |= getBit(value,  5, 6);
  bs |= getBit(value,  2, 7);

  float r = float(rs)/255.0;
  float g = float(gs)/255.0;
  float b = float(bs)/255.0;


  TP_GLSL_GLFRAGCOLOR = vec4(r, g, b, 1.0);
}
