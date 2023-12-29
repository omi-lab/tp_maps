#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec2 inVertex;

TP_GLSL_OUT_V vec2 coord_tex;

uniform mat4 frameMatrix;

void main()
{
  gl_Position = frameMatrix * vec4((inVertex * 2.0) - 1.0 , 0.0, 1.0);
  coord_tex = (gl_Position.xy+1.0)/2.0;
}
