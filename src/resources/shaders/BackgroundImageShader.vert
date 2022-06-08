/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec2 inVertex;

uniform mat4 frameMatrix;
uniform mat4 matrix;

/*TP_GLSL_OUT_V*/vec2 coord_tex;

void main()
{
  gl_Position = frameMatrix * vec4((inVertex * 2.0) - 1.0 , 0.0, 1.0);

  vec4 p = matrix * vec4(gl_Position.xy, 0.0, 1.0);
  vec2 v = p.xy / p.w;
  v = (v+1.0)/2.0;
  v.y = 1.0-v.y;
  coord_tex = v;
}
