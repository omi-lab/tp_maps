#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec4 inColor;
TP_GLSL_IN_V vec3 inPosition;
TP_GLSL_IN_V vec3 inOffset;
TP_GLSL_IN_V vec2 inTexture;

TP_GLSL_OUT_V vec2 coord_tex;
TP_GLSL_OUT_V vec4 picking;

uniform mat4 matrix;
uniform vec2 scaleFactor;
uniform uint pickingID;

void main()
{
  gl_Position = (matrix * vec4(inPosition, 1.0));
  gl_Position = vec4(gl_Position.xyz * (1.0/gl_Position.w), 1.0) + vec4(inOffset.x*scaleFactor.x, inOffset.y*scaleFactor.y, inOffset.z, 0.0);
  coord_tex = inTexture;
  uint id = pickingID + (uint(gl_VertexID)/4u);
  uint r = (id & 0x000000FFu) >>  0u;
  uint g = (id & 0x0000FF00u) >>  8u;
  uint b = (id & 0x00FF0000u) >> 16u;
  picking = vec4(r,g,b,255.0)/255.0;
}
