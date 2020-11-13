/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec3 lightVector0;
/*TP_GLSL_IN_F*/vec3 eyeNormal;
/*TP_GLSL_IN_F*/vec2 texCoordinate;
/*TP_GLSL_IN_F*/vec4 multColor;

uniform sampler2D textureSampler;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate)*multColor;
  if(/*TP_GLSL_GLFRAGCOLOR*/.a < 0.01)
    discard;
}
