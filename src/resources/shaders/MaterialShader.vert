$TP_VERT_SHADER_HEADER$

$TP_GLSL_IN_V$vec3 inVertex;
$TP_GLSL_IN_V$vec3 inNormal;
$TP_GLSL_IN_V$vec3 inTangent;
$TP_GLSL_IN_V$vec3 inBitangent;
$TP_GLSL_IN_V$vec2 inTexture;

uniform mat4 m;
uniform mat4 mvp;
uniform mat4 worldToLight0;

uniform vec3 cameraOrigin_world;

$TP_GLSL_OUT_V$vec2 uv_tangent;
$TP_GLSL_OUT_V$vec3 fragPos_tangent;
$TP_GLSL_OUT_V$vec3 cameraOrigin_tangent;
$TP_GLSL_OUT_V$vec3 light0Direction_tangent;

$TP_GLSL_OUT_V$vec4 fragPos_light0;

void main()
{
  vec3 light0Direction_world = vec3(-0.276, 0.276, -0.92);

  // Calculate the Tangent,Bitangent,Normal matrix
  // We use this to convert world coords into tangent space coords
  vec3 tangentCameraSpace   = (mat3(m) * inTangent  );
  vec3 bitangentCameraSpace = (mat3(m) * inBitangent);
  vec3 normalCameraSpace    = (mat3(m) * inNormal   );
  mat3 TBN = transpose(mat3(tangentCameraSpace, bitangentCameraSpace, normalCameraSpace));

  gl_Position = mvp * vec4(inVertex, 1.0);

  vec4 fragPos_world = m * vec4(inVertex, 1.0);
  fragPos_light0 = (vec4(0.5, 0.5, 0.5, 1) * ((worldToLight0 * m) * vec4(inVertex, 1.0))) + vec4(0.5, 0.5, 0.5, 0);

  uv_tangent = inTexture;
  light0Direction_tangent = TBN * light0Direction_world;
  cameraOrigin_tangent = TBN * cameraOrigin_world;
  fragPos_tangent = TBN * fragPos_world.xyz;
}
