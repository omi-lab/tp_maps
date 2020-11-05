/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

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
  float depth = /*TP_GLSL_TEXTURE*/(depthSampler, texCoordinate).x;
  vec3 viewCoord = clipToView(texCoordinate, depth);

  float occlusions=0.0;
  float count=0.0f;

  for(float x=-1.0; x<=1.0; x+=0.2)
  {
    for(float y=-1.0; y<=1.0; y+=0.2)
    {
      for(float z=-1.0; z<=1.0; z+=0.2)
      {
        vec4 clipCoord = projectionMatrix * vec4(viewCoord+vec3(x*0.01, y*0.01, z*0.01), 1.0);
        clipCoord /= clipCoord.w;
        clipCoord.xyz = (clipCoord.xyz*0.5) + 0.5;

        if(clipCoord.x>=0.0 && clipCoord.y>=0.0 && clipCoord.x<=1.0 && clipCoord.y<=1.0)
        {
          float d = /*TP_GLSL_TEXTURE*/(depthSampler, clipCoord.xy).x;
          vec3 viewCoord2 = clipToView(clipCoord.xy, d);

          if(clipCoord.z>d && distance2(viewCoord, viewCoord2)<0.01)
            occlusions++;
          count++;
        }
      }
    }
  }

  occlusions = max(0.0, occlusions-(count/2.0));

  float dim = (clamp((1.0 - occlusions/count)*1.0, 0.0, 1.0)+1.0) / 2.0;
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE*/(textureSampler, texCoordinate).xyz*dim, 1.0);



  // float d = /*TP_GLSL_TEXTURE*/(depthSampler, texCoordinate).x;
  // float d = occlusions/count;
  // float d = length(viewCoord.xyz);
  // float d = clamp((1.0 - occlusions/count)*3.0, 0.4, 1.0);
  // /*TP_GLSL_GLFRAGCOLOR*/ = vec4(d, d, d, 1.0f);
}
