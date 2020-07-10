$TP_VERT_SHADER_HEADER$

$TP_GLSL_IN_V$vec3 inVertex;
$TP_GLSL_IN_V$vec3 inNormal;
$TP_GLSL_IN_F$vec3 inTangent;
$TP_GLSL_IN_F$vec3 inBitangent;
$TP_GLSL_IN_V$vec2 inTexture;
//$TP_GLSL_IN_V$vec3 positionWorldspace;

uniform mat4 m;
//uniform mat4 v;
//uniform mat4 p;
uniform mat3 mv;
uniform mat4 mvp;
//uniform mat4 vp;

$TP_GLSL_OUT_V$vec3 lightVector0;
$TP_GLSL_OUT_V$vec3 EyeNormal;
$TP_GLSL_OUT_V$vec2 texCoordinate;
$TP_GLSL_OUT_V$vec3 normal;
$TP_GLSL_OUT_V$vec3 fragPos;

void main()
{
  gl_Position = mvp * vec4(inVertex, 1.0);
  fragPos = mat3(m)*inVertex;
  lightVector0 = vec3(0.0, 0.0, 1.0);
  normal = mat3(m)*inNormal;
  texCoordinate = inTexture;


  vec3 normalCameraSpace    = mv * inNormal;
  vec3 tangentCameraSpace   = mv * inTangent;
  vec3 bitangentCameraSpace = mv * inBitangent;

  mat3 TBN = transpose(mat3(
    tangentCameraSpace,
    bitangentCameraSpace,
    normalCameraSpace
  ));

  // LightDirection_tangentspace = TBN * LightDirection_cameraspace;
  // EyeDirection_tangentspace =  TBN * EyeDirection_cameraspace;



}
