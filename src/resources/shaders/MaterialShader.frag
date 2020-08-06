/*TP_FRAG_SHADER_HEADER*/

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

  vec2 spotLightUV;
  vec2 spotLightWH;
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
uniform sampler2D spotLightTexture;

uniform Material material;
uniform float picking;
uniform vec4 pickingID;

/*TP_GLSL_IN_F*/vec3 fragPos_world;

/*TP_GLSL_IN_F*/vec2 uv_tangent;
/*TP_GLSL_IN_F*/vec3 cameraOrigin_tangent;
/*TP_GLSL_IN_F*/vec3 fragPos_tangent;

/*LIGHT_FRAG_VARS*/

/*TP_GLSL_GLFRAGCOLOR_DEF*/

LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0, 1);
  float diff = (cosTheta+light.diffuseTranslate)*light.diffuseScale;
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
  float bias = dot(norm, normalize(-lightDirection_tangent));
  if(bias>0.0)
  {
    bias = 0.001;//max(0.0001, (1.0 - bias)*0.0005);
    vec2 texelSize = 2.0 / textureSize(lightTexture, 0);
    float biasedDepth = min(fragPos_light.z-bias,1.0);
    for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
      {
        vec2 coord = fragPos_light.xy + (vec2(x, y)*texelSize);
        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
        {
          shadow += 1.0;
        }
        else
        {
          float lightDepth = /*TP_GLSL_TEXTURE*/(lightTexture, coord).x;
          shadow += (lightDepth<biasedDepth)?0.0:1.0;
        }
      }
    }
    shadow /= 9.0;
  }

  r.diffuse *= shadow;
  r.specular *= shadow;

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
  vec3 shadowTex = vec3(0.0, 0.0, 0.0);
  float bias = dot(norm, normalize(-lightDirection_tangent));
  if(bias>0.0 && fragPos_light.z>0.0 && fragPos_light.z<1.0)
  {
    bias = max(0.0001, (1.0 - bias)*0.0005);
    vec2 texelSize = 2.0 / textureSize(lightTexture, 0);
    float biasedDepth = min(fragPos_light.z-bias,1.0);
    for(int x = -1; x <= 1; ++x)
    {
      for(int y = -1; y <= 1; ++y)
      {
        vec2 coord = fragPos_light.xy + (vec2(x, y)*texelSize);
        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
        {
          shadow += 0.0;
        }
        else
        {
          float lightDepth = /*TP_GLSL_TEXTURE*/(lightTexture, coord).x;
          shadow += (lightDepth<biasedDepth)?0.0:1.0;
        }
      }
    }
    shadow /= 9.0;    

    vec2 spotTexCoord = (fragPos_light.xy*light.spotLightWH) + light.spotLightUV;
    shadowTex = /*TP_GLSL_TEXTURE*/(spotLightTexture, spotTexCoord).xyz * shadow;
  }

  // Attenuation
  float distance    = length(light.position - fragPos_world);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  r.diffuse *= shadowTex;
  r.specular *= shadowTex;

  r.ambient  *= attenuation;
  r.diffuse  *= attenuation;
  r.specular *= attenuation;

  return r;
}

void main()
{
  vec3  ambientTex = /*TP_GLSL_TEXTURE*/( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = /*TP_GLSL_TEXTURE*/( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = /*TP_GLSL_TEXTURE*/(specularTexture, uv_tangent).xyz;
  vec3     bumpTex = /*TP_GLSL_TEXTURE*/(    bumpTexture, uv_tangent).xyz;

  vec3 norm = normalize(bumpTex*2-1);

  vec3 ambient  = vec3(0,0,0);
  vec3 diffuse  = vec3(0,0,0);
  vec3 specular = vec3(0,0,0);

  /*LIGHT_FRAG_CALC*/

  ambient  *= (ambientTex+material.ambient);
  diffuse  *= (diffuseTex+material.diffuse);
  specular *= (specularTex+material.specular);

  vec3 result = ambient + diffuse + specular;
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(result, material.alpha);
  /*TP_GLSL_GLFRAGCOLOR*/ = (picking*pickingID) + ((1.0-picking)*/*TP_GLSL_GLFRAGCOLOR*/);

  if(/*TP_GLSL_GLFRAGCOLOR*/.a<0.01)
    discard;
}
