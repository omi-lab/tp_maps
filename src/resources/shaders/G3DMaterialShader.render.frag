/*TP_FRAG_SHADER_HEADER*/

struct Material
{
  float useAmbient;
  float useDiffuse;
  float useNdotL;
  float useAttenuation;
  float useShadow;
  float useLightMask;
  float useReflection;

  bool rayVisibilityShadowCatcher;

  float albedoScale;

  float albedoBrightness;
  float albedoContrast;
  float albedoGamma;
  float albedoHue;
  float albedoSaturation;
  float albedoValue;
  float albedoFactor;
};

struct Light
{
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  float diffuseScale;

  float constant;
  float linear;
  float quadratic;

  float spotLightBlend;

  float near;
  float far;

  vec3 offsetScale;

  float orthoRadius;
  float fov;
};

struct LightResult
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

uniform sampler2D rgbaTexture;
uniform sampler2D normalsTexture;
uniform sampler2D rmttrTexture;

uniform Material material;

uniform vec2 txlSize;
uniform float discardOpacity;

uniform mat4 m;
uniform mat4 mv;
uniform mat4 mvp;
uniform mat4 v;
uniform mat4 mInv;

uniform vec3 cameraOrigin_world;

/*TP_GLSL_IN_F*/vec3 fragPos_world;

/*TP_GLSL_IN_F*/vec3 outNormal;

/*TP_GLSL_IN_F*/vec2 uv_tangent;
vec3 fragPos_tangent;
vec3 cameraOrigin_tangent;

vec3 albedo;
float roughness;
float roughness2;
float metalness;
float transmission;
float transmissionRoughness;

vec3 F0;
vec3 surfaceToCamera;

/*TP_GLSL_IN_F*/vec3 normal_view;

/*LIGHT_FRAG_VARS*/

/*POST_VARS*/

const int shadowSamples=/*TP_SHADOW_SAMPLES*/;
const float totalShadowSamples=float(((shadowSamples*2)+1) * ((shadowSamples*2)+1));

const float pi = 3.14159265;

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

//See MaterialShader.cpp for documentation.

// Fast HSV conversion source: https://stackoverflow.com/a/17897228
// All components are in the range [0…1], INCLUDING HUE.
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

// All components are in the range [0…1], INCLUDING HUE.
vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//##################################################################################################
float lineariseDepth(float depth, float near, float far)
{
  // Scale the depth back into world coords.
  return near * far / (far + depth * (near - far));
}

//##################################################################################################
float shadowMapDepth(highp sampler2D shadowMap, vec2 coords, float near, float far)
{
  vec2 pixelPos = (coords/txlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl).r, near, far);
  float brTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(txlSize.x, 0.0)).r, near, far);
  float tlTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(0.0, txlSize.y)).r, near, far);
  float trTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + txlSize).r, near, far);

  float mixA = mix(blTxl, tlTxl, fracPart.y);
  float mixB = mix(brTxl, trTxl, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

//##################################################################################################
float shadowMapMinDepth(highp sampler2D shadowMap, vec2 coords, float near, float far)
{
  vec2 pixelPos = (coords/txlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl).r, near, far);
  float brTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(txlSize.x, 0.0)).r, near, far);
  float tlTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(0.0, txlSize.y)).r, near, far);
  float trTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + txlSize).r, near, far);

  return min(min(blTxl, brTxl), min(tlTxl, trTxl));
}

//##################################################################################################
float sampleShadowMapLinear2D(highp sampler2D shadowMap, vec2 coords, float compareLight, float compareDark, float near, float far)
{
  return smoothstep(compareLight, compareDark, shadowMapDepth(shadowMap, coords, near, far));
}

//##################################################################################################
float shadowMapDepthAbsGradient(highp sampler2D shadowMap, vec2 coords, float near, float far)
{
  vec2 pixelPos = (coords/txlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl).r, near, far);
  float brTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(txlSize.x, 0.0)).r, near, far);
  float tlTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + vec2(0.0, txlSize.y)).r, near, far);
  float trTxl = lineariseDepth(/*TP_GLSL_TEXTURE_2D*/(shadowMap, startTxl + txlSize).r, near, far);

  return max(max(abs(brTxl-blTxl), abs(trTxl-tlTxl)), max(abs(tlTxl-blTxl), abs(trTxl-brTxl)));
}

//##################################################################################################
#ifndef NO_TEXTURE3D
float sampleShadowMapLinear3D(highp sampler3D shadowMap, vec2 coords, float compareLight, float compareDark, float level, float near, float far)
{
  vec2 pixelPos = (coords/txlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(/*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(startTxl, level)).r, near, far);
  float brTxl = lineariseDepth(/*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(startTxl + vec2(txlSize.x, 0.0), level)).r, near, far);
  float tlTxl = lineariseDepth(/*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(startTxl + vec2(0.0, txlSize.y), level)).r, near, far);
  float trTxl = lineariseDepth(/*TP_GLSL_TEXTURE_3D*/(shadowMap, vec3(startTxl + txlSize, level)).r, near, far);

  float mixA = mix(blTxl, tlTxl, fracPart.y);
  float mixB = mix(brTxl, trTxl, fracPart.y);

  return smoothstep(compareLight, compareDark, mix(mixA, mixB, fracPart.x));
}
#endif

//##################################################################################################
vec2 computeLightOffset(Light light, int offsetIdx)
{
  return lightOffsets[offsetIdx].xy * light.offsetScale.xy;
}

//##################################################################################################
vec3 lightPosToTexture(vec4 fragPos_light, vec2 offset, mat4 proj)
{
  vec4 fp = proj * (fragPos_light + vec4(offset,0.0,0.0));
  return (vec3(0.5, 0.5, 0.5) * (fp.xyz / fp.w) + vec3(0.5, 0.5, 0.5));
}

//##################################################################################################
float calcGGXDist(vec3 norm, vec3 halfV, float roughness2)
{
  float normDotLight = clamp(dot(norm, halfV), 0.0, 1.0) ;
  float normDotLight2 = normDotLight * normDotLight;
  float fDen = normDotLight2 * roughness2 + (1.0-normDotLight2);

  return roughness2/(pi*fDen * fDen);
}

//##################################################################################################
float chiGGX(float f)
{
  return f > 0.0 ? 1.0 : 0.0 ;
}

//##################################################################################################
float calcGGXGeom(vec3 surfaceToCamera, vec3 norm, vec3 lightViewHalfVector, float roughness2)
{
  float fViewerDotLightViewHalf = clamp(dot(surfaceToCamera, lightViewHalfVector), 0.0, 1.0) ;
  //float fChi = step(0.0, fViewerDotLightViewHalf / clamp(dot(surfaceToCamera, norm), 0.0, 1.0));
  float fChi = chiGGX(fViewerDotLightViewHalf / clamp(dot(surfaceToCamera, norm), 0.0, 1.0));
  fViewerDotLightViewHalf *= fViewerDotLightViewHalf;
  float fTan2 = (1.0 - fViewerDotLightViewHalf) / fViewerDotLightViewHalf;

  return (fChi * 2.0) / (1.0 + sqrt(1.0 + roughness2 * fTan2)) ;
}

//##################################################################################################
float geometrySchlickGGX(float NdotV, float k)
{
  float nom   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return nom / denom;
}

//##################################################################################################
float geometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx1 = geometrySchlickGGX(NdotV, k);
  float ggx2 = geometrySchlickGGX(NdotL, k);

  return ggx1 * ggx2;
}

//##################################################################################################
vec3 calcFresnel(vec3 halfV, vec3 norm, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0-max(0.0, dot(halfV, norm)), 5.0);
}

//##################################################################################################
LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, highp sampler2D lightTexture, vec3 uv)
{
  LightResult r;

  // Shadow
  float shadow = totalShadowSamples;
  float nDotL = dot(norm, -lightDirection_tangent);

  if(nDotL>0.0 && uv.z>0.0 && uv.z<1.0)
  {
    float linearDepth = lineariseDepth(uv.z, light.near, light.far);
    float bias = clamp((1.0-nDotL)*3.0, 0.1, 3.0) * linearDepth * linearDepth * 0.0004;
    float biasedDepth = linearDepth - bias;

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv.xy + (vec2(x, y)*txlSize);
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float extraBias = bias*(abs(float(x))+abs(float(y)));
          shadow -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, light.near, light.far);
        }
      }
    }
  }

  shadow /= totalShadowSamples;

  r.ambient = material.useAmbient * light.ambient * albedo;
  vec3 surfaceToLight  = -lightDirection_tangent;
  vec3 halfV = normalize(surfaceToCamera + surfaceToLight);

  // Attenuation
  //float distance    = length(light.position - fragPos_world);
  //float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  // No attenuation for directional lights.
  vec3 radiance = light.diffuseScale * light.diffuse;

  // Cook-Torrance BRDF
  float NDF = calcGGXDist(norm, halfV, roughness2);
  float G   = geometrySmith(norm, surfaceToCamera, surfaceToLight, roughness);
  vec3  F   = calcFresnel(halfV, norm, F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metalness;
  kD = mix(vec3(1.0,1.0,1.0), kD, material.useDiffuse);

  vec3  numerator   = NDF * G * F;
  //float denominator = 4.0 * max(dot(norm, surfaceToCamera), 0.0) * max(dot(norm, surfaceToLight), 0.0);
  float denominator = 4.0 * max(dot(norm, surfaceToCamera), 0.0);
  vec3  specular    = numerator / max(denominator, 0.001);

  // add to outgoing radiance Lo
  float NdotL = mix(1.0, max(dot(norm, surfaceToLight), 0.0), material.useNdotL);
  vec3 localLightAmount = radiance * NdotL * shadow;
  r.diffuse = kD * albedo * localLightAmount;
  r.specular = specular * localLightAmount;

  return r;
}

//##################################################################################################
float maskLight(Light light, vec3 uv, float shadow)
{
  float mask = 0.0;
  if(light.spotLightBlend>0.0001 && uv.x>=0.0 && uv.x<=1.0 && uv.y>=0.0 && uv.y<=1.0)
  {
    float l = length(uv.xy*2.0-1.0);
    mask = 1.0-clamp((l-(1.0-light.spotLightBlend))/light.spotLightBlend, 0.0, 1.0);
  }

  if(material.rayVisibilityShadowCatcher)
    return max((1.0-mask)*totalShadowSamples, shadow);
  else
    return mix(1.0, mask, material.useLightMask) * shadow;
}

//##################################################################################################
float spotLightSampleScale(float d_receiver, float d_blocker, Light light)
{
  // calculate effective ight size
  float w_penumbra = light.offsetScale.x*(d_receiver - d_blocker)/d_blocker;

  // calculate width of penumbra in shadow depth map texture coordinates at rendered pixel
  // total width is d_receiver*2*atan(fov/2)
  w_penumbra = w_penumbra/(2.0f*atan(0.5*light.fov)*d_receiver);

  // convert to pixel coordinates
  w_penumbra /= txlSize.x;
    
  // normalize to a per-sample value
  return w_penumbra/float(2*shadowSamples+1);
}

//##################################################################################################
float spotLightSampleShadow2D(vec3 norm, Light light, vec3 lightDirection_tangent, highp sampler2D lightTexture, vec3 uv)
{
  float lightLevel = totalShadowSamples;
  float nDotL = dot(norm, -lightDirection_tangent);

  if(nDotL>0.0 && uv.z>0.0 && uv.z<1.0)
  {
#if 0
    // for testing against no shadow filtering
    if(0 == shadowSamples)
    {
      float linearDepth = lineariseDepth(uv.z, light.near, light.far);
      float bias = 0.001f; //clamp((1.0-nDotL)*3.0, 0.1, 3.0) * linearDepth * linearDepth * 0.0950; // Original 0.0004
      float biasedDepth = linearDepth - bias;

      for(int x = -shadowSamples; x <= shadowSamples; ++x)
      {
        for(int y = -shadowSamples; y <= shadowSamples; ++y)
        {
          vec2 coord = uv.xy + (vec2(x, y)*txlSize);
          if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
          {
            float extraBias = bias*(abs(float(x))+abs(float(y)));
            lightLevel -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, light.near, light.far);
          }
        }
      }

      return maskLight(light, uv, lightLevel);
    }
#endif

    float bias = 0.01f; // + min(0.01f, shadowMapDepthAbsGradient(lightTexture, uv.xy, light.near, light.far));
    float linearDepth = lineariseDepth(uv.z, light.near, light.far);
    float biasedDepth = linearDepth - bias;

#if 0
    {
      // check that bias is sufficient
      float depth = shadowMapMinDepth(lightTexture, uv.xy, light.near, light.far);
      if(depth > biasedDepth)
        return .3f*totalShadowSamples;
      else
        return min(0.9, 0.5 + (biasedDepth-depth)*100.f)*totalShadowSamples;
    }
#endif   

    // use nominal low blocker depth value to estimate sample scale to include shadows.
    // We generally have objects close to the area being shadowed, so choose a nominal depth "close"
    // to the current depth.
    float nominalBlockerDepth = 0.8f*linearDepth; //light.near + 0.8f*(light.far - light.near);
    float sampleScale = spotLightSampleScale(linearDepth, nominalBlockerDepth, light);

    // use sample scale that implicitly sets a maximum size for the shadow filter
    float nSamplesXY = float(1+2*shadowSamples);
    sampleScale = max(1.f/nSamplesXY, min(sampleScale, 50.f/nSamplesXY));

    // estimate blocker depth
    float totWeight = 0.00001f; // make sure the weight is never zero
    float totWeightedShadowDepth = totWeight*linearDepth;

    // when calculating the blocker depth, weight pixels more at the centre
    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 thisOffset = vec2(x, y)*sampleScale*(1.f + 0.f*float(x*x+y*y)/totalShadowSamples)*txlSize;
        vec2 coord = uv.xy + thisOffset;
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float depth = shadowMapMinDepth(lightTexture, coord, light.near, light.far);

          float sumAbsXY = abs(float(x))+abs(float(y));
          float extraBias = bias*sampleScale*sumAbsXY;
          float thisShadow = 1.0f-smoothstep(biasedDepth-extraBias, linearDepth/*-extraBias*/, depth);
          float weight = thisShadow*pow(0.3f, sumAbsXY);
          
          totWeightedShadowDepth += weight*depth;
          totWeight += weight;
        }
      }
    }

    if(totWeight > 0.f)
    {
      // calculate averaged shadow depth
      float d_blocker = totWeightedShadowDepth/totWeight;
      sampleScale = spotLightSampleScale(linearDepth, d_blocker, light);

      // put reasonable limits on the size of the shadow filter
      sampleScale = max(0.8f/nSamplesXY, min(sampleScale, 50.f/nSamplesXY));

      // use a smaller bias now because we linearly interpolate the depth values
      bias = 0.001f; // + min(0.001f, shadowMapDepthAbsGradient(lightTexture, uv.xy, light.near, light.far));
      {
        for(int x = -shadowSamples; x <= shadowSamples; ++x)
          for(int y = -shadowSamples; y <= shadowSamples; ++y)
          {
            vec2 coord = uv.xy + (vec2(x, y)*sampleScale*txlSize);
            if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
            {
              float extraBias = bias*sampleScale*(abs(float(x))+abs(float(y)));
              lightLevel -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, light.near, light.far);
            }
          }
      }
    }

    return maskLight(light, uv, lightLevel);
  }

  return lightLevel*(material.rayVisibilityShadowCatcher?1.0:0.0);
}

//##################################################################################################
#ifndef NO_TEXTURE3D
float spotLightSampleShadow3D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler3D lightTexture, vec3 uv, float level)
{
  float shadow = totalShadowSamples;
  float nDotL = dot(norm, -lightDirection_tangent);

  if(nDotL>0.0 && uv.z>0.0 && uv.z<1.0)
  {
    float linearDepth = lineariseDepth(uv.z, light.near, light.far);
    float bias = 0.001f; //clamp((1.0-nDotL)*3.0, 0.1, 3.0) * linearDepth * linearDepth * 0.0950;
    float biasedDepth = linearDepth - bias;

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv.xy + (vec2(x, y)*txlSize*(nDotL));
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float extraBias = bias*(abs(float(x))+abs(float(y)));
          shadow -= 1.0-sampleShadowMapLinear3D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, level, light.near, light.far);
        }
      }
    }
    return maskLight(light, uv, shadow);
  }

  return shadow*(material.rayVisibilityShadowCatcher?1.0:0.0);
}
#endif

//##################################################################################################
LightResult spotLight(vec3 norm, Light light, vec3 lightDirection_tangent, vec3 fragPos_light, float shadow)
{
  LightResult r;

  r.ambient = material.useAmbient * light.ambient * albedo;

  vec3 surfaceToLight  = -lightDirection_tangent;
  vec3 halfV = normalize(surfaceToCamera + surfaceToLight);

  // Attenuation
  float distance    = length(light.position - fragPos_world);
  // Use a simplified attenuation function.
  //float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  float attenuation = 1.0 / (0.1 * distance);
  if (attenuation > 1.0)
    attenuation = 1.0;
  // Use an arbitrary value to scale up the light intensity (coming from diffuseScale parameter).
  vec3  maxRadiance = 3.0 * light.diffuseScale * light.diffuse;
  vec3  radiance    = mix(maxRadiance, maxRadiance * attenuation, material.useAttenuation);

  if (light.offsetScale.x > 1.0)
    radiance /= 4.18 * pow(light.offsetScale.x, 3.0); // 4/3*pi = 4.18

  // Cook-Torrance BRDF
  float NDF = calcGGXDist(norm, halfV, roughness2);
  float G   = geometrySmith(norm, surfaceToCamera, surfaceToLight, roughness);
  vec3  F   = calcFresnel(halfV, norm, F0);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metalness;
  kD = mix(vec3(1.0), kD, material.useDiffuse);

  vec3  numerator   = NDF * G * F;
  //float denominator = 4.0 * max(dot(norm, surfaceToCamera), 0.0) * max(dot(norm, surfaceToLight), 0.0);
  float denominator = 4.0 * max(dot(norm, surfaceToCamera), 0.0);
  vec3  specular    = numerator / max(denominator, 0.001);

  float NdotL = mix(1.0, max(dot(norm, surfaceToLight), 0.0), material.useNdotL);
  vec3 localLightAmount = radiance * NdotL * shadow;
  r.diffuse = kD * albedo * localLightAmount;
  r.specular = specular * localLightAmount;

  return r;
}

//##################################################################################################
mat3 transposeMat3(mat3 inMatrix)
{
  vec3 i0 = inMatrix[0];
  vec3 i1 = inMatrix[1];
  vec3 i2 = inMatrix[2];

  return mat3(vec3(i0.x, i1.x, i2.x), vec3(i0.y, i1.y, i2.y), vec3(i0.z, i1.z, i2.z));
}

//##################################################################################################
mat4 transposeIntoMat4(vec3 i0, vec3 i1, vec3 i2)
{
  return mat4(vec4(i0.x, i1.x, i2.x, 0.0),
              vec4(i0.y, i1.y, i2.y, 0.0),
              vec4(i0.z, i1.z, i2.z, 0.0),
              vec4(0.0, 0.0, 0.0, 1.0));
}

//##################################################################################################
void main()
{
  vec4     rgbaTex = /*TP_GLSL_TEXTURE_2D*/(    rgbaTexture, uv_tangent);
  vec3  normalsTex = /*TP_GLSL_TEXTURE_2D*/( normalsTexture, uv_tangent).xyz;
  vec4    rmttrTex = /*TP_GLSL_TEXTURE_2D*/(    rmttrTexture, uv_tangent);

  //Note: GammaCorrection
  rgbaTex.xyz = pow(rgbaTex.xyz, vec3(2.2));

  vec3 norm = normalize(normalsTex*2.0-1.0);

  albedo = rgbaTex.xyz * material.albedoScale;

  float contrast = 1.0 + material.albedoContrast;
  float brightness = material.albedoBrightness - material.albedoContrast * 0.5;

  albedo.r = max(contrast * albedo.r + brightness, 0.0);
  albedo.g = max(contrast * albedo.g + brightness, 0.0);
  albedo.b = max(contrast * albedo.b + brightness, 0.0);

  albedo = pow(albedo, vec3(material.albedoGamma));

  vec3 originalAlbedo = albedo;
  vec3 hsvAlbedo = rgb2hsv(albedo); // .r = hue, .g = saturation, .b = value

  hsvAlbedo.r = mod(hsvAlbedo.r + material.albedoHue + 0.5, 1.0);
  hsvAlbedo.g = clamp(hsvAlbedo.g * material.albedoSaturation, 0.0, 1.0);
  hsvAlbedo.b *= material.albedoValue;

  albedo = hsv2rgb(hsvAlbedo);

  // Clamp color to prevent negative values cauzed by oversaturation.
  albedo.r = max(albedo.r, 0.0);
  albedo.g = max(albedo.g, 0.0);
  albedo.b = max(albedo.b, 0.0);

  albedo = mix(originalAlbedo, albedo, material.albedoFactor);

  roughness = max(0.001, rmttrTex.x);
  roughness2 = roughness*roughness;
  metalness = rmttrTex.y;
  transmission = rmttrTex.z;
  transmissionRoughness = rmttrTex.w;

  vec3 ambient  = vec3(0,0,0);
  vec3 diffuse  = vec3(0,0,0);
  vec3 specular = vec3(0,0,0);

  // Calculate the TBN matrix used to transform between world and tangent space.
  vec3 n = normalize(outNormal);
  vec3 t1 = cross(vec3(1,0,0), outNormal);
  vec3 t2 = cross(vec3(0,1,0), outNormal);
  vec3 t = normalize((dot(t1, t1)>dot(t2,t2))?t1:t2);
  vec3 b = cross(n, t);
  t = cross(b, n);

  mat3 m3 = mat3(m);
  mat3 TBN = mat3(m3*t, m3*b, m3*n);
  mat3 invTBN = transposeMat3(TBN);
  mat3 TBNv = mat3(v) * TBN;

  mat4 worldToTangent = transposeIntoMat4(t, b, n) * mInv;

  {
    vec4 a = worldToTangent * vec4(cameraOrigin_world, 1.0);
    cameraOrigin_tangent = a.xyz/a.w;
  }

  {
    vec4 a = worldToTangent * vec4(fragPos_world, 1.0);
    fragPos_tangent = a.xyz/a.w;
  }

  F0 = mix(vec3(0.04), albedo, metalness);
  surfaceToCamera = normalize(cameraOrigin_tangent-fragPos_tangent);

  float accumulatedShadow = 1.0;
  float numShadows = 0.0;

  /*LIGHT_FRAG_CALC*/

  float alpha = rgbaTex.a;
  // Use transparency to display transmission and transmissionRoughness.
  if(transmissionRoughness > 0.1)
    transmission *= 0.5 * (1.0 - transmissionRoughness);
  float minAlpha = 0.2;
  // For non white material, use a higher alpha value = less transparency.
  if(rgbaTex.xyz != vec3(1.0))
    minAlpha = 0.5;
  if(transmission > 0.1)
    alpha = minAlpha + (1.0 - minAlpha) * (1.0 - transmission);

  float shininess = metalness;

  vec3 normal = TBNv*norm;

  /*POST*/

  if(material.rayVisibilityShadowCatcher)
  {
    ambient = vec3(0.0);
    diffuse = vec3(0.0);
    specular = vec3(0.0);
    alpha = clamp(1.0 - accumulatedShadow, 0.0, 0.8);
  }

  writeFragment(ambient, diffuse, specular, normal, alpha, vec3(1,1,1), shininess);
}
