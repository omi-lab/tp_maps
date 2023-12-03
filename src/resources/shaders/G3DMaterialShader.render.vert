/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;
/*TP_GLSL_IN_V*/vec3 inNormal;
/*TP_GLSL_IN_V*/vec3 inTangent;
/*TP_GLSL_IN_V*/vec2 inTexture;

uniform mat4 m;
uniform mat4 mv;
uniform mat4 mvp;
uniform mat4 v;
uniform mat3 uvMatrix;

/*TP_GLSL_OUT_V*/vec3 outNormal;
/*TP_GLSL_OUT_V*/vec3 outTangent;

uniform vec3 cameraOrigin_world;
/*TP_GLSL_OUT_V*/vec3 fragPos_world;

/*TP_GLSL_OUT_V*/vec2 uv_tangent;

/*TP_GLSL_OUT_V*/vec3 normal_view;

/*LIGHT_VERT_VARS*/

void main()
{
  outNormal    = inNormal;
  outTangent   = inTangent;

  gl_Position = mvp * vec4(inVertex, 1.0);

  fragPos_world = (m * vec4(inVertex, 1.0)).xyz;

  vec3 uv = uvMatrix * vec3(inTexture, 1.0f);
  uv_tangent = uv.xy;

/*LIGHT_VERT_CALC*/

  normal_view = mat3(mv) * inNormal;
}
