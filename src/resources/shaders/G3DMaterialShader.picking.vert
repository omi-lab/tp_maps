/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;

uniform mat4 mvp;

void main()
{
  gl_Position = mvp * vec4(inVertex, 1.0);
}
