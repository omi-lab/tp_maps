#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;

uniform sampler2D noiseSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;

/*AO_FRAG_VARS*/

uniform vec3 ssaoKernel[N_SAMPLES];

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

//##################################################################################################
vec3 clipToView(vec2 xy_tex, float depth, mat4 invProjectionMatrix)
{
  vec4 coord_view = invProjectionMatrix * vec4((xy_tex*2.0)-1.0, depth*2.0-1.0, 1.0);
  return coord_view.xyz / coord_view.w;
}

//##################################################################################################
vec2 testBuffer2D(vec3 coord_view, vec3 samplePos_view, mat4 projectionMatrix, mat4 invProjectionMatrix, sampler2D depthSampler)
{
  vec4 samplePos_clip = projectionMatrix * vec4(samplePos_view, 1.0);
  samplePos_clip /= samplePos_clip.w;
  samplePos_clip.xyz = (samplePos_clip.xyz*0.5) + 0.5;

  if(samplePos_clip.x>=0.0 && samplePos_clip.y>=0.0 && samplePos_clip.x<=1.0 && samplePos_clip.y<=1.0)
  {
    float d = TP_GLSL_TEXTURE_2D(depthSampler, samplePos_clip.xy).x;
    vec3 coord_view2 = clipToView(samplePos_clip.xy, d, invProjectionMatrix);

    if(samplePos_clip.z>(d+bias))
    {
      float dim = 1.0-clamp(distance(samplePos_view, coord_view2) / radius, 0.0, 1.0);
      if(dim>0.1)
        return vec2(dim, 0.0);
    }
    else
      return vec2(0.0, 0.0);
  }

  return vec2(1.0, 1.0);
}

void main()
{
  vec4 color;

  vec2 noiseScale = (1.0 / pixelSize) / 4.0;

  // AO calc
  float depth = TP_GLSL_TEXTURE_2D(depthSampler   , coord_tex).x;
  vec3 fragPos   = clipToView(coord_tex, depth, invProjectionMatrix);
  vec3 normal    = TP_GLSL_TEXTURE_2D(normalsSampler , coord_tex).xyz;
  vec3 randomVec = TP_GLSL_TEXTURE_2D(noiseSampler, coord_tex * noiseScale).xyz;

  vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN       = mat3(tangent, bitangent, normal);

  float occlusions = 0.0;
  for(int i = 0; i < N_SAMPLES; ++i)
  {
      // get sample position
      vec3 samplePos = TBN * ssaoKernel[i]; // from tangent to view-space
      samplePos = fragPos + samplePos * radius;

      vec2 occlusion = vec2(1.0);
      occlusion = min(testBuffer2D(fragPos, samplePos, projectionMatrix, invProjectionMatrix, depthSampler), occlusion);

      occlusions += occlusion.x - occlusion.y;
  }

  float dim = 1.0 - (occlusions / float(N_SAMPLES));

  TP_GLSL_GLFRAGCOLOR = vec4(dim);
}
