$TP_VERT_SHADER_HEADER$

uniform mat4 matrix;

$TP_GLSL_IN_V$vec3 position;

void main()
{
  gl_Position=matrix*vec4(position, 1.0);
}
