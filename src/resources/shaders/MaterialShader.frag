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

  float constant;
  float linear;
  float quadratic;
};

struct LightResult
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform sampler2D ambientTexture;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D bumpTexture;

uniform Material material;
uniform float picking;
uniform vec4 pickingID;

$TP_GLSL_IN_F$vec3 fragPos_world;

$TP_GLSL_IN_F$vec2 uv_tangent;
$TP_GLSL_IN_F$vec3 cameraOrigin_tangent;
$TP_GLSL_IN_F$vec3 fragPos_tangent;

uniform sampler2D light0Texture;
$TP_GLSL_IN_F$vec3 light0Direction_tangent;
uniform Light light0;
$TP_GLSL_IN_F$vec4 fragPos_light0;

uniform sampler2D light1Texture;
$TP_GLSL_IN_F$vec3 light1Direction_tangent;
uniform Light light1;
$TP_GLSL_IN_F$vec4 fragPos_light1;

$TP_GLSL_GLFRAGCOLOR_DEF$

LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0, 1);
  float diff = max((cosTheta+light.diffuseTranslate)*light.diffuseScale, 0);
  r.diffuse = light.diffuse * diff;

  // Specular
  vec3 incidenceVector = normalize(lightDirection_tangent);
  vec3 reflectionVector = reflect(incidenceVector, norm);
  vec3 surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  r.specular = specularCoefficient * light.specular;

  // Shadow
  float shadow = 0.0;
  float bias = 0.0009;//max(0.0005, (1.0-cosTheta)*0.05);
  vec2 texelSize = 2.0 / textureSize(lightTexture, 0);
  float biasedDepth = min(fragPos_light.z-bias,1.0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float lightDepth = $TP_GLSL_TEXTURE$(lightTexture, fragPos_light.xy + (vec2(x, y)*texelSize)).x;
      shadow += (lightDepth<biasedDepth)?0.0:1.0;
    }
  }
  shadow /= 9.0;

  r.diffuse *= shadow;
  r.specular *= shadow;

  return r;
}

LightResult pointLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0, 1);
  float diff = max((cosTheta+light.diffuseTranslate)*light.diffuseScale, 0);
  r.diffuse = light.diffuse * diff;

  // Specular
  vec3 incidenceVector = normalize(lightDirection_tangent);
  vec3 reflectionVector = reflect(incidenceVector, norm);
  vec3 surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  r.specular = specularCoefficient * light.specular;

  // Shadow
  float shadow = 0.0;
  float bias = 0.0009;//max(0.0005, (1.0-cosTheta)*0.05);
  vec2 texelSize = 2.0 / textureSize(lightTexture, 0);
  float biasedDepth = min(fragPos_light.z-bias,1.0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float lightDepth = $TP_GLSL_TEXTURE$(lightTexture, fragPos_light.xy + (vec2(x, y)*texelSize)).x;
      shadow += (lightDepth<biasedDepth)?0.0:1.0;
    }
  }
  shadow /= 9.0;

  float constant=1.0;
  float linear=0.1;
  float quadratic=0.1;

  // attenuation
  float distance    = length(light.position - fragPos_world);
  float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
  //float attenuation = 1.0 / (1.0 + 0.1*distance + 0.01*distance*distance);

  r.diffuse *= shadow;
  r.specular *= shadow;

  r.ambient  *= attenuation;
  r.diffuse  *= attenuation;
  r.specular *= attenuation;

  return r;
}

LightResult spotLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0, 1);
  float diff = max((cosTheta+light.diffuseTranslate)*light.diffuseScale, 0);
  r.diffuse = light.diffuse * diff;

  // Specular
  vec3 incidenceVector = normalize(lightDirection_tangent);
  vec3 reflectionVector = reflect(incidenceVector, norm);
  vec3 surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);
  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));
  float specularCoefficient = pow(cosAngle, material.shininess);
  r.specular = specularCoefficient * light.specular;

  // Shadow
  float shadow = 0.0;
  float bias = 0.0009;//max(0.0005, (1.0-cosTheta)*0.05);
  vec2 texelSize = 2.0 / textureSize(lightTexture, 0);
  float biasedDepth = min(fragPos_light.z-bias,1.0);
  for(int x = -1; x <= 1; ++x)
  {
    for(int y = -1; y <= 1; ++y)
    {
      float lightDepth = $TP_GLSL_TEXTURE$(lightTexture, fragPos_light.xy + (vec2(x, y)*texelSize)).x;
      shadow += (lightDepth<biasedDepth)?0.0:1.0;
    }
  }
  shadow /= 9.0;

  float constant=1.0;
  float linear=0.1;
  float quadratic=0.1;

  // attenuation
  float distance    = length(light.position - fragPos_world);
  float attenuation = 1.0 / (constant + linear * distance + quadratic * (distance * distance));
  //float attenuation = 1.0 / (1.0 + 0.1*distance + 0.01*distance*distance);

  r.diffuse *= shadow;
  r.specular *= shadow;

  r.ambient  *= attenuation;
  r.diffuse  *= attenuation;
  r.specular *= attenuation;

  return r;
}

void main()
{
  vec3  ambientTex = $TP_GLSL_TEXTURE$( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = $TP_GLSL_TEXTURE$( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = $TP_GLSL_TEXTURE$(specularTexture, uv_tangent).xyz;
  vec3     bumpTex = $TP_GLSL_TEXTURE$(    bumpTexture, uv_tangent).xyz;

  vec3 norm = normalize(bumpTex*2-1);

  vec3 ambient  = vec3(0,0,0);
  vec3 diffuse  = vec3(0,0,0);
  vec3 specular = vec3(0,0,0);

  {
    LightResult r = directionalLight(norm, light0, light0Direction_tangent, light0Texture, fragPos_light0);
    ambient  += r.ambient;
    diffuse  += r.diffuse;
    specular += r.specular;
  }

  {
    LightResult r = spotLight(norm, light1, light1Direction_tangent, light1Texture, fragPos_light1);
    ambient  += r.ambient;
    diffuse  += r.diffuse;
    specular += r.specular;
  }

  ambient  *= (ambientTex+material.ambient);
  diffuse  *= (diffuseTex+material.diffuse);
  specular *= (specularTex+material.specular);

  vec3 result = ambient + diffuse + specular;
  $TP_GLSL_GLFRAGCOLOR$ = vec4(result, material.alpha);
  $TP_GLSL_GLFRAGCOLOR$ = (picking*pickingID) + ((1.0-picking)*$TP_GLSL_GLFRAGCOLOR$);

  if($TP_GLSL_GLFRAGCOLOR$.a<0.01)
    discard;
}
