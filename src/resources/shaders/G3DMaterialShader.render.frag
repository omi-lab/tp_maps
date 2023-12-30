#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D
#define TP_GLSL_TEXTURE_3D

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

  float fov;
};

struct LightResult
{
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

TP_GLSL_IN_F vec3 fragPos_world;
TP_GLSL_IN_F vec4 outTBNq;
TP_GLSL_IN_F vec2 uv_tangent;

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
  
mat4 worldToTangent;
vec3 cameraOrigin_tangent;
vec3 fragPos_tangent;

vec3 albedo;
float roughness;
float roughness2;
float metalness;
float transmission;
float transmissionRoughness;

vec3 F0;
vec3 surfaceToCamera;

vec2 invTxlSize;

#pragma replace LIGHT_FRAG_VARS

uniform int shadowSamples;

float totShadowSamples()
{
  return float(((shadowSamples*2)+1) * ((shadowSamples*2)+1));
}

const float pi = 3.14159265;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF
#pragma replace TP_WRITE_FRAGMENT

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
  vec2 pixelPos = (coords*invTxlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(TP_GLSL_TEXTURE_2D(shadowMap, startTxl).r, near, far);
  float brTxl = lineariseDepth(TP_GLSL_TEXTURE_2D(shadowMap, startTxl + vec2(txlSize.x, 0.0)).r, near, far);
  float tlTxl = lineariseDepth(TP_GLSL_TEXTURE_2D(shadowMap, startTxl + vec2(0.0, txlSize.y)).r, near, far);
  float trTxl = lineariseDepth(TP_GLSL_TEXTURE_2D(shadowMap, startTxl + txlSize).r, near, far);

  float mixA = mix(blTxl, tlTxl, fracPart.y);
  float mixB = mix(brTxl, trTxl, fracPart.y);

  return mix(mixA, mixB, fracPart.x);
}

//##################################################################################################
float sampleShadowMapLinear2D(highp sampler2D shadowMap, vec2 coords, float compareLight, float compareDark, float near, float far)
{
  return smoothstep(compareLight, compareDark, shadowMapDepth(shadowMap, coords, near, far));
}

//##################################################################################################
#ifndef NO_TEXTURE3D
float sampleShadowMapLinear3D(highp sampler3D shadowMap, vec2 coords, float compareLight, float compareDark, float level, float near, float far)
{
  vec2 pixelPos = (coords*invTxlSize) - 0.5;
  vec2 fracPart = fract(pixelPos);
  vec2 startTxl = (pixelPos-fracPart) * txlSize;

  float blTxl = lineariseDepth(TP_GLSL_TEXTURE_3D(shadowMap, vec3(startTxl, level)).r, near, far);
  float brTxl = lineariseDepth(TP_GLSL_TEXTURE_3D(shadowMap, vec3(startTxl + vec2(txlSize.x, 0.0), level)).r, near, far);
  float tlTxl = lineariseDepth(TP_GLSL_TEXTURE_3D(shadowMap, vec3(startTxl + vec2(0.0, txlSize.y), level)).r, near, far);
  float trTxl = lineariseDepth(TP_GLSL_TEXTURE_3D(shadowMap, vec3(startTxl + txlSize, level)).r, near, far);

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
  vec4 fp = proj * (fragPos_light + vec4(offset,0.0,0.0)); // range is now [-1,1] in each axis
  return (vec3(0.5, 0.5, 0.5) * (fp.xyz / fp.w) + vec3(0.5, 0.5, 0.5)); // transformed to range [0,1]
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
LightResult directionalLight(vec3 norm, Light light, vec3 lightDirection_tangent, highp sampler2D lightTexture, vec3 uv_light)
{
  LightResult r;

  // Shadow
  float shadow = totShadowSamples();
  float nDotL = dot(norm, -lightDirection_tangent);

  if(nDotL>0.0 && uv_light.z>0.0 && uv_light.z<1.0)
  {
    float linearDepth = lineariseDepth(uv_light.z, light.near, light.far);
    float bias = clamp((1.0-nDotL)*3.0, 0.1, 3.0) * linearDepth * linearDepth * 0.0004;
    float biasedDepth = linearDepth - bias;

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv_light.xy + (vec2(x, y)*txlSize);
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float extraBias = bias*(abs(float(x))+abs(float(y)));
          shadow -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, light.near, light.far);
        }
      }
    }
  }

  shadow /= totShadowSamples();

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
float maskLight(Light light, vec3 uv_light, float shadow)
{
  float mask = 0.0;
  if(light.spotLightBlend>0.0001 && uv_light.x>=0.0 && uv_light.x<=1.0 && uv_light.y>=0.0 && uv_light.y<=1.0)
  {
    float l = length(uv_light.xy*2.0-1.0);
    mask = 1.0-clamp((l-(1.0-light.spotLightBlend))/light.spotLightBlend, 0.0, 1.0);
  }

  if(material.rayVisibilityShadowCatcher)
    return max((1.0-mask)*totShadowSamples(), shadow);
  else
    return mix(1.0, mask, material.useLightMask) * shadow;
}

//##################################################################################################
float lightTotalSizeXY(float depth, Light light)
{
  return 2.0f*tan(0.5f*light.fov)*depth;
}

//##################################################################################################
float spotLightSampleScale(float d_receiver, float d_blocker, Light light, float offsetScale)
{
  // calculate effective ight size at the render pixel position
  float w_penumbra = offsetScale*(d_receiver - d_blocker)/d_blocker;

  // calculate width of penumbra in shadow depth map texture coordinates at rendered pixel
  // total width is d_receiver*2*tan(fov/2)
  float fiddleFactor = 2.0f; // to be tuned to match the Blender soft shadows
  w_penumbra = fiddleFactor*w_penumbra/lightTotalSizeXY(d_receiver, light);

  // convert to pixel coordinates
  return w_penumbra*invTxlSize.x;
}

//##################################################################################################
float rand(vec2 co){
  return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

//##################################################################################################
float spotLightSampleShadow2D(vec3 norm, Light light, vec3 lightDirection_tangent, highp sampler2D lightTexture, vec3 uv_light)
{
  float totalShadowSamples = totShadowSamples();
  float lightLevel = totalShadowSamples;

  float nDotL = dot(norm, -lightDirection_tangent);
  if(nDotL>0.0 && uv_light.z>0.0 && uv_light.z<1.0)
  {
    // bias depends on depth and tan of angle = sqrt(1 - cos^2)/cos
    float linearDepth = lineariseDepth(uv_light.z, light.near, light.far);
    float tana = sqrt(1.f-nDotL*nDotL)/nDotL;
    float bias = (0.0003f + 0.000f*tana)*linearDepth;
    float biasedDepth = linearDepth - bias;

    // apply simple loop if no shadow filtering
    if(0 == shadowSamples)
    {
      if(uv_light.x>=0.0 && uv_light.x<=1.0 && uv_light.y>=0.0 && uv_light.y<=1.0)
        lightLevel -= 1.0-sampleShadowMapLinear2D(lightTexture, uv_light.xy, biasedDepth-bias, linearDepth-bias, light.near, light.far);

      return maskLight(light, uv_light, lightLevel);
    }

    // use nominal low blocker depth value to estimate sample scale to include shadows.
    // We generally have objects close to the area being shadowed, so choose a nominal depth "close"
    // to the current depth. We choose to reduce the nominal blocker depth when
    // the light is low to the horizon, indicated by nDotL being close to 0.
    float nominalBlockerDepth = 0.8f/*max(0.8f - 0.2f*tana, 0.9f)*/*linearDepth;

    // calculate gradient of depth w.r.t. pixel position
    vec2 uvxydx = dFdx(uv_light.xy);
    vec2 uvxydy = dFdy(uv_light.xy);
    float depthdx = dFdx(linearDepth);
    float depthdy = dFdy(linearDepth);

    // calculate sample scale based on the nominal blocker depth
    float nSamplesXY = float(1+2*shadowSamples);
    float sampleScale = clamp(spotLightSampleScale(linearDepth, nominalBlockerDepth, light, light.offsetScale.x), 1.0f, max(5.0f, 0.05f/(txlSize.x*tan(0.5f*light.fov))))/nSamplesXY;

    vec2 randomOffset = vec2(rand(uv_light.xy)-0.5f, rand(uv_light.yz)-0.5f);

    // we have locally shadow map coordinates sx, sy, and we want to calculate gradxy = ( d(depth)/d(sx) )
    //                                                                                  ( d(depth)/d(sy) )
    // From the derivatives we are given we can estimate
    //                            d(depth)      d(depth)
    // depth(sx,sy) = depth0 + sx*-------- + sy*--------
    //                              d(sx)         d(sy)
    //
    // We can separate the sx/sy components from the uvxt derivatives as
    //   a*uvxydx + b*uvxydy = (1), c*uvxydx + d*uvxydy = (0)
    //                         (0)                        (1)
    // 
    // This is solved by a 2x2 matrix equation, then we can calculate
    //   depth(sx,sy) = sx*(a*depthdx + b*depthdy) + sy*(c*depthdx + d*depthdy)
    //
    // 
    mat2 A = mat2(uvxydx, uvxydy);
    mat2 Ai = inverse(A);
    vec2 depthGradXY = txlSize*vec2(Ai[0][0]*depthdx + Ai[1][0]*depthdy, Ai[0][1]*depthdx + Ai[1][1]*depthdy);

    // calculate blocker depth as average shadow-weighted depth around current pixel
    float totWeight = 0.0f; // make sure the weight is never zero
    float totWeightedShadowDepth = 0.f;
    float divN = 1.0f/(nSamplesXY*nSamplesXY);
    for(int x = -shadowSamples; x <= shadowSamples; ++x)
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coffset = sampleScale*(vec2(x, y) + randomOffset);
        vec2 coord = uv_light.xy + coffset*txlSize;
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float depth = shadowMapDepth(lightTexture, coord, light.near, light.far);
          float extraBias = 2.f*bias*(abs(coffset.x) + abs(coffset.y));
          float depthShift = dot(coffset, depthGradXY);
          float thisShadow = 1.0f-smoothstep(biasedDepth+depthShift-extraBias, linearDepth+depthShift-extraBias, depth);

          // weight shadow less at the edge of the window
          float weight = thisShadow*pow(0.001f, float(x*x+y*y)*divN);
          totWeightedShadowDepth += weight*depth;
          totWeight += weight;
        }
      }

    if(totWeight > 0.001f*nSamplesXY*nSamplesXY)
    {
      // calculate averaged shadow depth
      float d_blocker = totWeightedShadowDepth/totWeight;

      // put reasonable limits on the size of the shadow filter
      sampleScale = clamp(spotLightSampleScale(linearDepth, d_blocker, light, light.offsetScale.x), 1.0f, 0.1f*invTxlSize.x)/nSamplesXY;

      for(int x = -shadowSamples; x <= shadowSamples; ++x)
        for(int y = -shadowSamples; y <= shadowSamples; ++y)
        {
          vec2 coffset = sampleScale*(vec2(x, y) + randomOffset);
          vec2 coord = uv_light.xy + coffset*txlSize;
          if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
          {
            float extraBias = 2.0f*bias*(abs(coffset.x) + abs(coffset.y));
            float depthShift = dot(coffset, depthGradXY);
            lightLevel -= 1.0-sampleShadowMapLinear2D(lightTexture, coord, biasedDepth+depthShift-extraBias, linearDepth+depthShift-extraBias, light.near, light.far);
          }
        }
    }

    return maskLight(light, uv_light, lightLevel);
  }

  return lightLevel*(material.rayVisibilityShadowCatcher?1.0:0.0);
}

//##################################################################################################
#ifndef NO_TEXTURE3D
float spotLightSampleShadow3D(vec3 norm, Light light, vec3 lightDirection_tangent, sampler3D lightTexture, vec3 uv_light, float level)
{
  float shadow = totShadowSamples();
  float nDotL = dot(norm, -lightDirection_tangent);

  if(nDotL>0.0 && uv_light.z>0.0 && uv_light.z<1.0)
  {
    float linearDepth = lineariseDepth(uv_light.z, light.near, light.far);
    float bias = 0.002f;
    float biasedDepth = linearDepth - bias;

    for(int x = -shadowSamples; x <= shadowSamples; ++x)
    {
      for(int y = -shadowSamples; y <= shadowSamples; ++y)
      {
        vec2 coord = uv_light.xy + (vec2(x, y)*txlSize*(nDotL));
        if(coord.x>=0.0 && coord.x<=1.0 && coord.y>=0.0 && coord.y<=1.0)
        {
          float extraBias = bias*(abs(float(x))+abs(float(y)));
          shadow -= 1.0-sampleShadowMapLinear3D(lightTexture, coord, biasedDepth-extraBias, linearDepth-extraBias, level, light.near, light.far);
        }
      }
    }
    return maskLight(light, uv_light, shadow);
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
mat4 transposeIntoMat4(mat3 i)
{
  return mat4(vec4(i[0][0], i[1][0], i[2][0], 0.0),
              vec4(i[0][1], i[1][1], i[2][1], 0.0),
              vec4(i[0][2], i[1][2], i[2][2], 0.0),
              vec4(0.0, 0.0, 0.0, 1.0));
}

//##################################################################################################
mat3 quaternionToMat3(vec4 q)
{
  float qxx = q.x * q.x;
  float qyy = q.y * q.y;
  float qzz = q.z * q.z;
  float qxz = q.x * q.z;
  float qxy = q.x * q.y;
  float qyz = q.y * q.z;
  float qwx = q.w * q.x;
  float qwy = q.w * q.y;
  float qwz = q.w * q.z;

  mat3 R;
  R[0][0] = 1.0f - 2.0f * (qyy +  qzz);
  R[0][1] = 2.0f * (qxy + qwz);
  R[0][2] = 2.0f * (qxz - qwy);

  R[1][0] = 2.0f * (qxy - qwz);
  R[1][1] = 1.0f - 2.0f * (qxx +  qzz);
  R[1][2] = 2.0f * (qyz + qwx);

  R[2][0] = 2.0f * (qxz + qwy);
  R[2][1] = 2.0f * (qyz - qwx);
  R[2][2] = 1.0f - 2.0f * (qxx +  qyy);
  return R;
}

//##################################################################################################
void main()
{
  vec4     rgbaTex = TP_GLSL_TEXTURE_2D(    rgbaTexture, uv_tangent);
  vec3  normalsTex = TP_GLSL_TEXTURE_2D( normalsTexture, uv_tangent).xyz;
  vec4    rmttrTex = TP_GLSL_TEXTURE_2D(    rmttrTexture, uv_tangent);

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
  mat3 m3 = mat3(m);
  mat3 TBN = quaternionToMat3(normalize(outTBNq));
  mat3 mTBN = m3*TBN;
  mat3 invmTBN = transposeMat3(mTBN);

  invTxlSize = vec2(1.f/txlSize.x, 1.f/txlSize.y);

  worldToTangent = transposeIntoMat4(TBN) * mInv;

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

#pragma replace LIGHT_FRAG_CALC

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

  /*POST*/

  if(material.rayVisibilityShadowCatcher)
  {
    ambient = vec3(0.0);
    diffuse = vec3(0.0);
    specular = vec3(0.0);
    alpha = clamp(1.0 - accumulatedShadow, 0.0, 0.8);
  }

  writeFragment(ambient, diffuse, specular, alpha);
}
