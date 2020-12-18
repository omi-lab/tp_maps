/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec2 inVertex;

/*TP_GLSL_OUT_V*/vec2 coord_tex;

void main()
{
  gl_Position = vec4((inVertex * 2.0) - 1.0 , 0.0, 1.0);
  coord_tex = inVertex;
}
