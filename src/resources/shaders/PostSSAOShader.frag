/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  float depth = /*TP_GLSL_TEXTURE*/(depthSampler, texCoordinate).x;
  vec4 viewCoord = invProjectionMatrix * vec4((texCoordinate*2.0)-1.0, depth*2.0-1.0, 1.0);

  viewCoord /= viewCoord.w;

  float occlusions=0.0;
  float count=0.0f;

  for(float x=-1.0; x<=1.0; x+=0.2)
  {
    for(float y=-1.0; y<=1.0; y+=0.2)
    {
      for(float z=-1.0; z<=1.0; z+=0.2)
      {
        vec4 clipCoord = projectionMatrix * (viewCoord+vec4(x*0.02, y*0.02, z*0.02, 0.0));
        clipCoord /= clipCoord.w;
        clipCoord.xyz = (clipCoord.xyz*0.5) + 0.5;

        if(clipCoord.x>=0.0 && clipCoord.y>=0.0 && clipCoord.x<=1.0 && clipCoord.y<=1.0)
        {
          float d = /*TP_GLSL_TEXTURE*/(depthSampler, clipCoord.xy).x;
          if(clipCoord.z>(d+0.00001))
            occlusions++;
          count++;
        }
      }
    }
  }

  float dim = clamp((1.0 - occlusions/count)*2.0, 0.1, 1.0);


  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE*/(textureSampler, texCoordinate).xyz*dim, 1.0);
}
