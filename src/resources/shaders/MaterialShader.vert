/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;
/*TP_GLSL_IN_V*/vec3 inNormal;
/*TP_GLSL_IN_V*/vec3 inTangent;
/*TP_GLSL_IN_V*/vec3 inBitangent;
/*TP_GLSL_IN_V*/vec2 inTexture;

uniform mat4 m;
uniform mat4 mvp;

uniform vec3 cameraOrigin_world;
/*TP_GLSL_OUT_V*/vec3 fragPos_world;

/*TP_GLSL_OUT_V*/vec2 uv_tangent;
/*TP_GLSL_OUT_V*/vec3 fragPos_tangent;
/*TP_GLSL_OUT_V*/vec3 cameraOrigin_tangent;

/*LIGHT_VERT_VARS*/

mat3 transposeMat3(mat3 inMatrix)
{
  vec3 i0 = inMatrix[0];
  vec3 i1 = inMatrix[1];
  vec3 i2 = inMatrix[2];

  return mat3(vec3(i0.x, i1.x, i2.x), vec3(i0.y, i1.y, i2.y), vec3(i0.z, i1.z, i2.z));
}

void main()
{
  // Calculate the Tangent,Bitangent,Normal matrix
  // We use this to convert world coords into tangent space coords
  vec3 tangentCameraSpace   = (mat3(m) * inTangent  );
  vec3 bitangentCameraSpace = (mat3(m) * inBitangent);
  vec3 normalCameraSpace    = (mat3(m) * inNormal   );
  mat3 TBN = transposeMat3(mat3(tangentCameraSpace, bitangentCameraSpace, normalCameraSpace));

  gl_Position = mvp * vec4(inVertex, 1.0);

  fragPos_world = (m * vec4(inVertex, 1.0)).xyz;

  uv_tangent = inTexture;

  /*LIGHT_VERT_CALC*/

  cameraOrigin_tangent = TBN * cameraOrigin_world;
  fragPos_tangent = TBN * fragPos_world;
}
