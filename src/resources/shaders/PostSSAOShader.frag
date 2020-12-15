/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;
uniform sampler2D specularSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform float near;
uniform float far;

uniform vec3 ssaoKernel[64];

/*TP_GLSL_GLFRAGCOLOR_DEF*/

vec3 clipToView(vec2 texXY, float depth)
{
  vec4 viewCoord = invProjectionMatrix * vec4((texXY*2.0)-1.0, depth*2.0-1.0, 1.0);
  return viewCoord.xyz / viewCoord.w;
}

float distance2(vec3 a, vec3 b)
{
  vec3 c = a - b;
  return dot(c, c);
}

void main()
{
  float depth    = /*TP_GLSL_TEXTURE_2D*/(depthSampler, texCoordinate).x;
  vec3 normal    = /*TP_GLSL_TEXTURE_2D*/(normalsSampler, texCoordinate).xyz;
  vec3 viewCoord = clipToView(texCoordinate, depth);

  vec3 rndA = vec3(0.4, 0.4, 0.0); //texture(texNoise, TexCoords * noiseScale).xyz;
  vec3 rndB = vec3(rndA.y, rndA.z, rndA.x); //texture(texNoise, TexCoords * noiseScale).xyz;
  vec3 randomVec = (abs(dot(rndA, normal)) > abs(dot(rndB, normal)))?rndB:rndA;

  vec3 t = cross(randomVec, normal);
  vec3 n = normal;
  vec3 b = cross(n, t);
  t = cross(n, b);

  mat3 TBN = mat3(t, b, n);

  float occlusions = 0.0;
  float radius=0.05;
  for(int i=0; i<64; i++)
  {
      // get sample position
      vec3 samplePos = TBN * ssaoKernel[i]; // from tangent to view-space
      samplePos = viewCoord + samplePos * radius;

      vec4 clipCoord = projectionMatrix * vec4(samplePos, 1.0);
      clipCoord /= clipCoord.w;
      clipCoord.xyz = (clipCoord.xyz*0.5) + 0.5;

      if(clipCoord.x>=0.0 && clipCoord.y>=0.0 && clipCoord.x<=1.0 && clipCoord.y<=1.0)
      {
        float d = /*TP_GLSL_TEXTURE_2D*/(depthSampler, clipCoord.xy).x;
        vec3 viewCoord2 = clipToView(clipCoord.xy, d);

        float bias=0.000025;
        if(clipCoord.z>(d+bias))
        {
          float rangeCheck = smoothstep(0.0, 1.0, radius / distance(viewCoord, viewCoord2));
          occlusions       += rangeCheck;
        }
        //        if(depth>d)
        //          occlusions++;
        //count++;
      }

  }

  float dim = 1.0 - (occlusions / 64.0);

  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate).xyz*dim, 1.0);

//  float depth = /*TP_GLSL_TEXTURE_2D*/(depthSampler, texCoordinate).x;
//  vec3 viewCoord = clipToView(texCoordinate, depth);

//  float occlusions=0.0;
//  float count=0.0f;

//  for(float x=-1.0; x<=1.01; x+=0.2)
//  {
//    for(float y=-1.0; y<=1.01; y+=0.2)
//    {
//      for(float z=-1.0; z<=1.01; z+=0.2)
//      {
//        vec4 clipCoord = projectionMatrix * vec4(viewCoord+vec3(x*0.1, y*0.1, z*0.1), 1.0);
//        clipCoord /= clipCoord.w;
//        clipCoord.xyz = (clipCoord.xyz*0.5) + 0.5;

//        if(clipCoord.x>=0.0 && clipCoord.y>=0.0 && clipCoord.x<=1.0 && clipCoord.y<=1.0)
//        {
//          float d = /*TP_GLSL_TEXTURE_2D*/(depthSampler, clipCoord.xy).x;
//          vec3 viewCoord2 = clipToView(clipCoord.xy, d);

//          if(clipCoord.z>d)
//            occlusions += 1.0 - min(distance(viewCoord, viewCoord2) * 0.1, 1.0);
//          count++;
//        }
//      }
//    }
//  }

//  occlusions = max(0.0, occlusions-(count/2.0));

//  float dim = clamp((1.0 - ((occlusions/count)*1.5)), 0.9, 1.0);
//  dim = pow(dim, 0.2);

//  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate).xyz*dim, 1.0);

  ///*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/(normalsSampler, texCoordinate);



  // float d = /*TP_GLSL_TEXTURE_2D*/(depthSampler, texCoordinate).x;
  // float d = occlusions/count;
  // float d = length(viewCoord.xyz);
  // float d = clamp((1.0 - ((occlusions/count)*2.0)), 0.0, 1.0);
  // /*TP_GLSL_GLFRAGCOLOR*/ = vec4(dim, dim, dim, 1.0f);
}
