/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec2 inTexture;

/*TP_GLSL_IN_F*/vec3 LightVector0;

uniform sampler2D textureSampler;
//uniform vec4 color;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  // /*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex);
  // if(/*TP_GLSL_GLFRAGCOLOR*/.a < 0.01)
  //   discard;

  float depth = gl_FragCoord.z;
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(depth, depth, depth, 1.0);
}
