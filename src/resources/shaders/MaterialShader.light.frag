/*TP_FRAG_SHADER_HEADER*/

uniform sampler2D rgbaTexture;
/*TP_GLSL_IN_F*/vec2 uv_tangent;

const float discardOpacity=0.8;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  vec4 rgbaTex = /*TP_GLSL_TEXTURE_2D*/(rgbaTexture, uv_tangent);

  if(rgbaTex.a<discardOpacity)
	discard;
  else
	/*TP_GLSL_GLFRAGCOLOR*/= vec4(gl_FragCoord.z, 0, 0, 1);
}
