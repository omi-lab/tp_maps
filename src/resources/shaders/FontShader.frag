/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;
uniform sampler2D textureSampler;
uniform vec4 color;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/ (textureSampler, coord_tex)*color;
  if(/*TP_GLSL_GLFRAGCOLOR*/.a < 0.01)
    discard;
}
