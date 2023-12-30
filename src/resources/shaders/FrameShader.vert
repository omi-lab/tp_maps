#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec3 inVertexP;
TP_GLSL_IN_V vec3 inVertexR;
TP_GLSL_IN_V vec4 inTBNq;
TP_GLSL_IN_V vec2 inTexture;

TP_GLSL_OUT_V vec3 lightVector0;
TP_GLSL_OUT_V vec4 eyeTBNq;
TP_GLSL_OUT_V vec2 coord_tex;
TP_GLSL_OUT_V vec4 multColor;

uniform mat4 matrix;
uniform vec3 scale;
uniform vec4 color;

void main()
{
  vec3 inVertex = inVertexP+(inVertexR*scale);
  gl_Position = matrix * vec4(inVertex, 1.0);
  lightVector0 = vec3(1.0, 1.0, 1.0);
  eyeTBNq = inTBNq;
  coord_tex = inTexture;
  multColor = color;
}
