/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;
/*TP_GLSL_IN_F*/vec4 picking;

uniform sampler2D textureSampler;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = picking;
  if(/*TP_GLSL_TEXTURE*/(textureSampler, texCoordinate).a < 0.001)
    discard;
}
