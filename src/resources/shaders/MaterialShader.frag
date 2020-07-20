$TP_FRAG_SHADER_HEADER$

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  float alpha;
};

struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float diffuseScale;
  float diffuseTranslate;
};

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D bumpTexture;
uniform sampler2D light0Texture;

uniform Material material;
uniform Light light;
uniform float picking;
uniform vec4 pickingID;

$TP_GLSL_IN_F$vec2 uv_tangent;
$TP_GLSL_IN_F$vec3 cameraOrigin_tangent;
$TP_GLSL_IN_F$vec3 fragPos_tangent;
$TP_GLSL_IN_F$vec3 light0Direction_tangent;

$TP_GLSL_IN_F$vec4 fragPos_light0;

$TP_GLSL_GLFRAGCOLOR_DEF$

void main()
{
  vec3  ambientTex = $TP_GLSL_TEXTURE$( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = $TP_GLSL_TEXTURE$( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = $TP_GLSL_TEXTURE$(specularTexture, uv_tangent).xyz;
  vec3     bumpTex = $TP_GLSL_TEXTURE$(    bumpTexture, uv_tangent).xyz;

  float light0Depth = $TP_GLSL_TEXTURE$( light0Texture, fragPos_light0.xy).x;

  vec3 norm = normalize(bumpTex*2-1);

  // Ambient
  vec3 ambient = light.ambient * (ambientTex+material.ambient);

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-light0Direction_tangent)), 0, 1);
  float diff = max((cosTheta+light.diffuseTranslate)*light.diffuseScale, 0);
  vec3 diffuse = light.diffuse * (diff * (diffuseTex+material.diffuse));

  // Specular
  vec3 incidenceVector = normalize(light0Direction_tangent);
  vec3 reflectionVector = reflect(incidenceVector, norm);
  vec3 surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  vec3 specular = specularCoefficient * light.specular * (specularTex+material.specular);

  vec3 result = ambient + diffuse + specular;
  $TP_GLSL_GLFRAGCOLOR$ = vec4(result, material.alpha);
  $TP_GLSL_GLFRAGCOLOR$ = (picking*pickingID) + ((1.0-picking)*$TP_GLSL_GLFRAGCOLOR$);

  $TP_GLSL_GLFRAGCOLOR$.x = (fragPos_light0.z>light0Depth)?0.0:1.0;
}
