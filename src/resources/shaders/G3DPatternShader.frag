#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D
#define TP_GLSL_TEXTURE_3D

TP_GLSL_IN_F vec2 texCoord;
uniform sampler2D rgbaTexture;
uniform float discardOpacity;
uniform int patternSelection;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

//##################################################################################################
const int PatternSelection_Ghosty = 0;

//##################################################################################################
float pattern_Ghosty(in vec2 p)
{
  float  line_count  = 8.;
  float  line_center = fract(( p.x + (1.0 - p.y) ) * line_count);
  return smoothstep(1.15, -0.15, abs(line_center - 0.5));
}

//##################################################################################################
void main()
{
  vec4  color   = TP_GLSL_TEXTURE_2D(rgbaTexture, texCoord);
  vec3  diffuse = vec3(1.0);
  float alpha   = 1.0;

  if(patternSelection == PatternSelection_Ghosty)
  {
    float mask = pattern_Ghosty(texCoord);
    diffuse    = vec3(1.0-mask) + color.rgb * mask;
    alpha      = max(color.a * mask, 0.001);
  }

  if(alpha<discardOpacity)
    discard;

  TP_GLSL_GLFRAGCOLOR = vec4(diffuse, alpha);
}
