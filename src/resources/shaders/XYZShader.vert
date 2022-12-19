/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;

uniform mat4 m;
uniform mat4 mvp;

/*TP_GLSL_OUT_V*/vec3 vertex;

void main()
{
  gl_Position = mvp * vec4(inVertex, 1.0);
  vertex = vec4(m * vec4(inVertex, 1.0)).xyz;
}
