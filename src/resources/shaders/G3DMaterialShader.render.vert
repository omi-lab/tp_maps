/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;
/*TP_GLSL_IN_V*/vec4 inTBNq;
/*TP_GLSL_IN_V*/vec2 inTexture;

uniform mat4 m;
uniform mat4 mv;
uniform mat4 mvp;
uniform mat4 v;
uniform mat3 uvMatrix;

/*TP_GLSL_OUT_V*/vec4 outTBNq;

uniform vec3 cameraOrigin_world;
/*TP_GLSL_OUT_V*/vec3 fragPos_world;

/*TP_GLSL_OUT_V*/vec2 uv_tangent;

/*TP_GLSL_OUT_V*/vec3 normal_view;

/*LIGHT_VERT_VARS*/

//##################################################################################################
vec3 quaternionToMat3Z(vec4 q)
{
  vec3 res;
  res.x = 2.0f * (q.x*q.z + q.w*q.y);
  res.y = 2.0f * (q.y*q.z - q.w*q.x);
  res.z = 1.0f - 2.0f * (q.x*q.x +  q.y*q.y);
  return res;
}

void main()
{
  outTBNq    = inTBNq;

  gl_Position = mvp * vec4(inVertex, 1.0);

  fragPos_world = (m * vec4(inVertex, 1.0f)).xyz;

  vec3 uv = uvMatrix * vec3(inTexture, 1.0f);
  uv_tangent = uv.xy;

/*LIGHT_VERT_CALC*/

  normal_view = mat3(mv) * quaternionToMat3Z(normalize(inTBNq));
}
