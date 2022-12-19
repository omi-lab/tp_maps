/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec3 vertex;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

//##################################################################################################
void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(vertex, 1.0);
}
