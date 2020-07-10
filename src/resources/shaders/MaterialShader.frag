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

$TP_GLSL_IN_F$vec3 fragPos;
$TP_GLSL_IN_F$vec3 normal;
$TP_GLSL_IN_F$vec2 texCoordinate;

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D bumpTexture;

uniform vec3 cameraOriginNear;
uniform vec3 cameraOriginFar;
uniform Material material;
uniform Light light;
uniform float picking;
uniform vec4 pickingID;

$TP_GLSL_IN_F$vec3 lightVector0;
$TP_GLSL_IN_F$vec3 EyeNormal;

$TP_GLSL_GLFRAGCOLOR_DEF$

void main()
{
  vec3 ambientTex = $TP_GLSL_TEXTURE$(ambientTexture, texCoordinate).xyz;
  vec3 diffuseTex = $TP_GLSL_TEXTURE$(diffuseTexture, texCoordinate).xyz;
  vec3 specularTex = $TP_GLSL_TEXTURE$(specularTexture, texCoordinate).xyz;
  vec3 bumpTex = $TP_GLSL_TEXTURE$(bumpTexture, texCoordinate).xyz;

  //vec3 norm = normalize(normal);
  vec3 norm = normalize(bumpTex * 2.0 - 1.0);

  // Ambient
  vec3 ambient = light.ambient * (ambientTex+material.ambient);

  // Diffuse
  float diff = max((dot(norm, lightVector0)+light.diffuseTranslate)*light.diffuseScale, 0.0);
  vec3 diffuse = light.diffuse * (diff * (diffuseTex+material.diffuse));

  // Specular
  vec3 incidenceVector = -lightVector0; //a unit vector
  vec3 reflectionVector = reflect(incidenceVector, norm); //also a unit vector
  vec3 surfaceToCamera = normalize(cameraOriginNear - cameraOriginFar); //also a unit vector
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  vec3 specular = specularCoefficient * light.specular * (specularTex+material.specular);

  vec3 result = ambient + diffuse + specular;
  $TP_GLSL_GLFRAGCOLOR$ = vec4(result, material.alpha);
  $TP_GLSL_GLFRAGCOLOR$ = (picking*pickingID) + ((1.0-picking)*$TP_GLSL_GLFRAGCOLOR$);
}
