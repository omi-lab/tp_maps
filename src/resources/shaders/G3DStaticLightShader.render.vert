#pragma replace TP_VERT_SHADER_HEADER
#define TP_GLSL_IN_V
#define TP_GLSL_OUT_V

TP_GLSL_IN_V vec3 inVertex;
TP_GLSL_IN_V vec4 inTBNq;
TP_GLSL_IN_V vec2 inTexture;

TP_GLSL_OUT_V vec4 outTBNq;
TP_GLSL_OUT_V vec3 fragPos_world;
TP_GLSL_OUT_V vec2 uv_tangent;

uniform mat4 m;
uniform mat4 mv;
uniform mat4 mvp;
uniform mat4 v;
uniform mat3 uvMatrix;

uniform vec3 cameraOrigin_world;

void main()
{
  outTBNq = inTBNq;
  gl_Position = mvp * vec4(inVertex, 1.0);

  fragPos_world = (m * vec4(inVertex, 1.0)).xyz;

  vec3 uv = uvMatrix * vec3(inTexture, 1.0f);
  uv_tangent = uv.xy;
}
