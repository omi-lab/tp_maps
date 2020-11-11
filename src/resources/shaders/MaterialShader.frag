/*TP_FRAG_SHADER_HEADER*/

struct Material
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
  float alpha;

  float ambientScale;
  float diffuseScale;
  float specularScale;
};

struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float diffuseScale;

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
uniform sampler2D alphaTexture;
uniform sampler2D bumpTexture;
uniform sampler2D spotLightTexture;

uniform Material material;

uniform float discardOpacity;

/*TP_GLSL_IN_F*/vec3 fragPos_world;

/*TP_GLSL_IN_F*/vec2 uv_tangent;
/*TP_GLSL_IN_F*/vec3 cameraOrigin_tangent;
/*TP_GLSL_IN_F*/vec3 fragPos_tangent;

/*LIGHT_FRAG_VARS*/

/*TP_GLSL_GLFRAGCOLOR_DEF*/

const int shadowSamples=1;
const float totalSadowSamples=float(((shadowSamples*2)+1) * ((shadowSamples*2)+1));

// Taken from: https://github.com/BennyQBD/3DEngineCpp/blob/master/res/shaders/sampling.glh
// https://youtu.be/yn5UJzMqxj0
float sampleShadowMap(sampler2D shadowMap, vec2 coords, float compare)
{
  return step(compare, /*TP_GLSL_TEXTURE*/(shadowMap, coords.xy).r);
}

// Taken from: https://github.com/BennyQBD/3DEngineCpp/blob/master/res/shaders/sampling.glh
// https://youtu.be/yn5UJzMqxj0
float sampleShadowMapLinear(sampler2D shadowMap, vec2 coords, float compare, vec2 texelSize)
{
  vec2 pixelPos = coords/texelSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTexel = (pixelPos - fracPart) * texelSize;

  float blTexel = sampleShadowMap(shadowMap, startTexel, compare);
  float brTexel = sampleShadowMap(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
  float tlTexel = sampleShadowMap(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
  float trTexel = sampleShadowMap(shadowMap, startTexel + texelSize, compare);

  float mixA = mix(blTexel, tlTexel, fracPart.y);
  float mixB = mix(brTexel, trTexel, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec2 lightTextureSize, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0.0, 1.0);
  float diff = cosTheta*light.diffuseScale;
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
    vec2 texelSize = vec2(2.0, 2.0) / lightTextureSize;
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
          shadow += sampleShadowMapLinear(lightTexture, coord, biasedDepth, texelSize);
        }
      }
    }
    shadow /= 9.0;
  }

  r.diffuse *= shadow;
  r.specular *= shadow;

  return r;
}

LightResult spotLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec2 lightTextureSize, vec4 fragPos_light)
{
  LightResult r;

  // Ambient
  r.ambient = light.ambient;

  // Diffuse
  float cosTheta = clamp(dot(norm, normalize(-lightDirection_tangent)), 0.0, 1.0);
  float diff = max(cosTheta*light.diffuseScale, 0.0);
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
    vec2 texelSize = vec2(1.0, 1.0) / lightTextureSize;
    float biasedDepth = min(fragPos_light.z-bias,1.0);
    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = fragPos_light.xy + (vec2(x, y)*texelSize);
        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
        {
          shadow += 0.0;
        }
        else
        {
          shadow += sampleShadowMapLinear(lightTexture, coord, biasedDepth, texelSize);
        }
      }
    }
    shadow /= totalSadowSamples;

    vec2 spotTexCoord = (fragPos_light.xy*light.spotLightWH) + light.spotLightUV;
    shadowTex = /*TP_GLSL_TEXTURE*/(spotLightTexture, spotTexCoord).xyz * shadow;
  }

  // Attenuation
  float distance    = length(light.position - fragPos_world);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

  //r.ambient  *= shadowTex;
  r.diffuse  *= shadowTex;
  r.specular *= shadowTex;

  //r.ambient  *= attenuation;
  r.diffuse  *= attenuation;
  r.specular *= attenuation;

  return r;
}

void main()
{
  vec3  ambientTex = /*TP_GLSL_TEXTURE*/( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = /*TP_GLSL_TEXTURE*/( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = /*TP_GLSL_TEXTURE*/(specularTexture, uv_tangent).xyz;
  float   alphaTex = /*TP_GLSL_TEXTURE*/(   alphaTexture, uv_tangent).x;
  vec3     bumpTex = /*TP_GLSL_TEXTURE*/(    bumpTexture, uv_tangent).xyz;

  vec3 norm = normalize(bumpTex*2.0-1.0);

  vec3 ambient  = vec3(0,0,0);
  vec3 diffuse  = vec3(0,0,0);
  vec3 specular = vec3(0,0,0);

  /*LIGHT_FRAG_CALC*/

  ambient  *= (ambientTex+material.ambient)   * material.ambientScale;
  diffuse  *= (diffuseTex+material.diffuse)   * material.diffuseScale;
  specular *= (specularTex+material.specular) * material.specularScale;

  vec3 result = ambient + diffuse + specular;

  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(result, material.alpha*alphaTex);

  if(/*TP_GLSL_GLFRAGCOLOR*/.a<discardOpacity)
    discard;
}
