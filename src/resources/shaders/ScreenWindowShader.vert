#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec2 inVertex;

TP_GLSL_OUT_V vec2 coord_tex;

uniform mat4 matrix;

void main()
{
  gl_Position = matrix * vec4((inVertex * 2.0) - 1.0 , 0.0, 1.0);
  coord_tex = inVertex;
}
