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

uniform vec2 texelSize;
uniform float discardOpacity;

/*TP_GLSL_IN_F*/vec3 fragPos_world;

/*TP_GLSL_IN_F*/vec2 uv_tangent;
/*TP_GLSL_IN_F*/vec3 cameraOrigin_tangent;
/*TP_GLSL_IN_F*/vec3 fragPos_tangent;

/*LIGHT_FRAG_VARS*/

/*TP_GLSL_GLFRAGCOLOR_DEF*/

const int shadowSamples=/*TP_SHADOW_SAMPLES*/;
const float totalSadowSamples=float(((shadowSamples*2)+1) * ((shadowSamples*2)+1));

// Taken from: https://github.com/BennyQBD/3DEngineCpp/blob/master/res/shaders/sampling.glh
// https://youtu.be/yn5UJzMqxj0

float sampleShadowMap2D(sampler2D shadowMap, vec2 coords, float compare)
{
  return step(compare, /*TP_GLSL_TEXTURE_2D*/(shadowMap, coords.xy).r);
}

float sampleShadowMapLinear2D(sampler2D shadowMap, vec2 coords, float compare)
{
  vec2 pixelPos = coords/texelSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTexel = (pixelPos - fracPart) * texelSize;

  float blTexel = sampleShadowMap2D(shadowMap, startTexel, compare);
  float brTexel = sampleShadowMap2D(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare);
  float tlTexel = sampleShadowMap2D(shadowMap, startTexel + vec2(0.0, texelSize.y), compare);
  float trTexel = sampleShadowMap2D(shadowMap, startTexel + texelSize, compare);

  float mixA = mix(blTexel, tlTexel, fracPart.y);
  float mixB = mix(brTexel, trTexel, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

float sampleShadowMap3D(sampler3D shadowMap, vec2 coords, float compare, float level)
{
  return step(compare, /*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(coords.xy, level)).r);
}

float sampleShadowMapLinear3D(sampler3D shadowMap, vec2 coords, float compare, float level)
{
  vec2 pixelPos = coords/texelSize + vec2(0.5);
  vec2 fracPart = fract(pixelPos);
  vec2 startTexel = (pixelPos - fracPart) * texelSize;

  float blTexel = sampleShadowMap3D(shadowMap, startTexel, compare, level);
  float brTexel = sampleShadowMap3D(shadowMap, startTexel + vec2(texelSize.x, 0.0), compare, level);
  float tlTexel = sampleShadowMap3D(shadowMap, startTexel + vec2(0.0, texelSize.y), compare, level);
  float trTexel = sampleShadowMap3D(shadowMap, startTexel + texelSize, compare, level);

  float mixA = mix(blTexel, tlTexel, fracPart.y);
  float mixB = mix(brTexel, trTexel, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

vec3 lightPosToTexture(vec4 fragPos_light, vec4 offset, mat4 proj)
{
  vec4 fp = proj * (fragPos_light+offset);
  return vec3((vec3(0.5, 0.5, 0.5) * (fp.xyz / fp.w)) + vec3(0.5, 0.5, 0.5));
}

LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec3 fragPos_light)
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
    float biasedDepth = min(fragPos_light.z-bias,1.0);
    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = fragPos_light.xy + (vec2(x, y)*texelSize);
        if(coord.x<0.0 || coord.x>1.0 || coord.y<0.0 || coord.y>1.0)
        {
          shadow += 1.0;
        }
        else
        {
          shadow += sampleShadowMapLinear2D(lightTexture, coord, biasedDepth);
        }
      }
    }
    shadow /= 9.0;
  }

  r.diffuse *= shadow;
  r.specular *= shadow;

  return r;
}

LightResult spotLight2D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler2D lightTexture, vec3 fragPos_light)
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
          shadow += sampleShadowMapLinear2D(lightTexture, coord, biasedDepth);
        }
      }
    }
    shadow /= totalSadowSamples;

    vec2 spotTexCoord = (fragPos_light.xy*light.spotLightWH) + light.spotLightUV;
    shadowTex = /*TP_GLSL_TEXTURE_2D*/(spotLightTexture, spotTexCoord).xyz * shadow;
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

float spotLightSampleShadow3D(vec3 norm, vec3 lightDirection_tangent, sampler3D lightTexture, vec3 fragPos_light, float level)
{
  float shadow = 0.0;

  float bias = dot(norm, normalize(-lightDirection_tangent));
  if(bias>0.0 && fragPos_light.z>0.0 && fragPos_light.z<1.0)
  {
    bias = max(0.0001, (1.0 - bias)*0.0005);
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
          shadow += sampleShadowMapLinear3D(lightTexture, coord, biasedDepth, level);
        }
      }
    }
  }

  return shadow;
}

LightResult spotLight3D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler3D lightTexture, vec3 fragPos_light, float shadow)
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
  vec2 spotTexCoord = (fragPos_light.xy*light.spotLightWH) + light.spotLightUV;
  vec3 shadowTex = /*TP_GLSL_TEXTURE_2D*/(spotLightTexture, spotTexCoord).xyz * shadow;

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
  vec3  ambientTex = /*TP_GLSL_TEXTURE_2D*/( ambientTexture, uv_tangent).xyz;
  vec3  diffuseTex = /*TP_GLSL_TEXTURE_2D*/( diffuseTexture, uv_tangent).xyz;
  vec3 specularTex = /*TP_GLSL_TEXTURE_2D*/(specularTexture, uv_tangent).xyz;
  float   alphaTex = /*TP_GLSL_TEXTURE_2D*/(   alphaTexture, uv_tangent).x;
  vec3     bumpTex = /*TP_GLSL_TEXTURE_2D*/(    bumpTexture, uv_tangent).xyz;

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
