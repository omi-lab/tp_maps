$TP_VERT_SHADER_HEADER$

$TP_GLSL_IN_V$vec3 inVertex;
$TP_GLSL_IN_V$vec3 inNormal;
$TP_GLSL_IN_V$vec3 inTangent;
$TP_GLSL_IN_V$vec3 inBitangent;
$TP_GLSL_IN_V$vec2 inTexture;

uniform mat4 m;
uniform mat4 mvp;
uniform vec3 cameraOrigin_world;

$TP_GLSL_OUT_V$vec2 uv_tangent;
$TP_GLSL_OUT_V$vec3 fragPos_tangent;
$TP_GLSL_OUT_V$vec3 cameraOrigin_tangent;
$TP_GLSL_OUT_V$vec3 light0Direction_tangent;

void main()
{
  vec3 light0Direction_world = vec3(0.0, 0.0, -1.0);

  // Calculate the Tangent,Bitangent,Normal matrix
  // We use this to convert world coords into tangent space coords
  vec3 tangentCameraSpace   = (m * vec4(inTangent  , 1)).xyz;
  vec3 bitangentCameraSpace = (m * vec4(inBitangent, 1)).xyz;
  vec3 normalCameraSpace    = (m * vec4(inNormal   , 1)).xyz;
  mat3 TBN = transpose(mat3(tangentCameraSpace, bitangentCameraSpace, normalCameraSpace));

  gl_Position = mvp * vec4(inVertex, 1.0);
  uv_tangent = inTexture;

  light0Direction_tangent = TBN * light0Direction_world;
  cameraOrigin_tangent = TBN * cameraOrigin_world;
  fragPos_tangent = TBN * (m*vec4(inVertex,1)).xyz;
}
