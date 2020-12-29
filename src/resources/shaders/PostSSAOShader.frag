/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;
uniform sampler2D specularSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform float near;
uniform float far;

const float discardOpacity=0.8;

/*AO_FRAG_VARS*/

uniform vec3 ssaoKernel[N_SAMPLES];

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

//##################################################################################################
float calcBias(float depth, mat4 projectionMatrix, mat4 invProjectionMatrix, float biasMeters)
{
  return 0.000025;
}

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
    float d = /*TP_GLSL_TEXTURE_2D*/(depthSampler, samplePos_clip.xy).x;
    vec3 coord_view2 = clipToView(samplePos_clip.xy, d, invProjectionMatrix);

    float bias=calcBias(d, projectionMatrix, invProjectionMatrix, 0.0001);
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

//##################################################################################################
vec2 testBuffer3D(vec3 coord_view, vec3 samplePos_view, mat4 projectionMatrix, mat4 invProjectionMatrix, sampler3D depthSampler)
{
  vec4 samplePos_clip = projectionMatrix * vec4(samplePos_view, 1.0);
  samplePos_clip /= samplePos_clip.w;
  samplePos_clip.xyz = (samplePos_clip.xyz*0.5) + 0.5;

  if(samplePos_clip.x>=0.0 && samplePos_clip.y>=0.0 && samplePos_clip.x<=1.0 && samplePos_clip.y<=1.0)
  {
    float d = /*TP_GLSL_TEXTURE_3D*/(depthSampler, vec3(samplePos_clip.xy, 0.0)).x;
    vec3 coord_view2 = clipToView(samplePos_clip.xy, d, invProjectionMatrix);

    float bias=calcBias(d, projectionMatrix, invProjectionMatrix, 0.0001);
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

//##################################################################################################
void main()
{
  float depth = /*TP_GLSL_TEXTURE_2D*/(depthSampler   , coord_tex).x;
  gl_FragDepth = depth;
  vec3 normal = normalize(/*TP_GLSL_TEXTURE_2D*/(normalsSampler , coord_tex).xyz);

  //The position of the current texel in view coords
  vec3 coord_view = clipToView(coord_tex, depth, invProjectionMatrix);

  vec3 rndA = vec3(1.0, 0.0, 0.0); //texture(texNoise, TexCoords * noiseScale).xyz;
  vec3 rndB = vec3(rndA.y, rndA.z, rndA.x);
  vec3 randomVec = (abs(dot(rndA, normal)) > abs(dot(rndB, normal)))?rndB:rndA;

  vec3 t = cross(randomVec, normal);
  vec3 n = normal;
  vec3 b = cross(n, t);
  t = cross(n, b);

  mat3 TBN = mat3(t, b, n);

  float occlusions = 0.0;
  for(int i=0; i<N_SAMPLES; i++)
  {
    vec2 occlusion = vec2(1.0);

    vec3 samplePos_view = TBN * ssaoKernel[i];
    samplePos_view = coord_view + samplePos_view * radius;

    /*AO_FRAG_CALC*/

    occlusions += occlusion.x - occlusion.y;
  }

  float dim = 1.0 - (occlusions / float(N_SAMPLES));

  vec3 ambient = /*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz*dim;

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  float alpha = 1.0;
  vec3 materialSpecular = vec3(0.0);
  float shininess = 0.0;

  writeFragment(ambient, diffuse, specular, normal, alpha, materialSpecular, shininess);
}
