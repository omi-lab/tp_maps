#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR

TP_GLSL_IN_F vec2 coord_tex;

const float discardOpacity=0.8;

uniform vec2 scaleFactor;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF
#pragma replace TP_WRITE_FRAGMENT

//##################################################################################################
void main()
{
  vec2 f = mod(coord_tex * scaleFactor, 2.0f);
  bool a = (f.x<1.0) ^^ (f.y<1.0);

  vec3 ambient = a?vec3(0.7,0.7,0.7):vec3(0.9,0.9,0.9);
  //vec3 ambient = vec3(coord_tex.x,coord_tex.y,0.0);

  //Note: GammaCorrection
  ambient = pow(ambient, vec3(2.2));

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  float alpha = 1.0;

  writeFragment(ambient, diffuse, specular, alpha);
}
