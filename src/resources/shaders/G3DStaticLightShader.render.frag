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
/*TP_GLSL_IN_F*/vec3 outTangent;

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

/*POST_VARS*/

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
LightResult directionalLight(vec3 norm, vec3 lightDirection_tangent)
{
  LightResult r;

  float nDotL = dot(norm, -lightDirection_tangent);

  r.ambient = material.useAmbient * albedo;

  vec3 surfaceToLight  = -lightDirection_tangent;
  vec3 halfV = normalize(surfaceToCamera + surfaceToLight);

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

  float radiance = 4.0;

  // add to outgoing radiance Lo
  float NdotL = mix(1.0, max(dot(norm, surfaceToLight), 0.0), material.useNdotL);
  vec3 localLightAmount = radiance * vec3(NdotL, NdotL, NdotL);
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

  // Calculate the TBN matrix used to transform between world and tangent space.
  vec3 b = cross(outNormal, outTangent);
  vec3 t = cross(b, outNormal);
  mat3 TBN = mat3(normalize(t), normalize(b), normalize(outNormal));

  mat3 m3 = mat3(m);
  mat3 mTBN = m3*TBN;
  mat3 invmTBN = transposeMat3(mTBN);
  mat3 vmTBNv = mat3(v) * mTBN;

  mat4 worldToTangent = transposeIntoMat4(TBN) * mInv;

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

  vec3 ldNormalized = normalize(invmTBN * vec3(0.0,0.0,-1.0));

  //directionalLight(norm, light%, ldNormalized, light%Texture, lightPosToTexture(fragPos_light%View, vec2(0,0), worldToLight%_proj));
  LightResult r = directionalLight(norm, ldNormalized);

  vec3 ambient  = r.ambient ;
  vec3 diffuse  = r.diffuse ;
  vec3 specular = r.specular;

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

  vec3 normal = vmTBN*norm;

  /*POST*/


  writeFragment(ambient, diffuse, specular, normal, alpha, vec3(1,1,1), shininess);
}
