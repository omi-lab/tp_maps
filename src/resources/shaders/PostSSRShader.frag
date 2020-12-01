/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;
uniform sampler2D specularSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

vec3 clipToView(vec2 texXY, float depth)
{
  vec4 viewCoord = invProjectionMatrix * vec4((texXY*2.0)-1.0, depth*2.0-1.0, 1.0);
  return viewCoord.xyz / viewCoord.w;
}

vec3 viewToClip(vec3 viewCoord)
{
  vec4 clipCoord = projectionMatrix * vec4(viewCoord, 1.0);
  return ((clipCoord.xyz / clipCoord.w)+1.0)/2.0;
}

float distance2(vec3 a, vec3 b)
{
  vec3 c = a - b;
  return dot(c, c);
}

vec3 fragColorR = vec3(0.0);
float count=0;
void takeSample(vec3 viewCoord, vec3 reflectRay, vec3 noise)
{
  reflectRay = normalize(reflectRay+noise);

  for(float i=0.01; i<1.0; i+=0.03)
  {
    vec3 fragPosReflecting = viewToClip(viewCoord + (reflectRay*i));
    if(fragPosReflecting.x<0.0 || fragPosReflecting.x>1.0 || fragPosReflecting.y<0.0 || fragPosReflecting.y>1.0)
      return;

    float fragDepth  = /*TP_GLSL_TEXTURE_2D*/(depthSampler, fragPosReflecting.xy).x;

    float delta = (fragPosReflecting.z-0.000001) - fragDepth;

    if(delta>0.0 && delta<0.001)
    {
      vec3 normalReflecting = normalize(/*TP_GLSL_TEXTURE_2D*/(normalsSampler, fragPosReflecting.xy).xyz);

      if(dot(reflectRay, normalReflecting) < 0.0)
      {
        fragColorR += /*TP_GLSL_TEXTURE_2D*/(textureSampler, fragPosReflecting.xy).xyz;// * max(0.0, (1.0-(i*2.0)));
        count+=1.0;
        break;
      }
    }
  }
}

void main()
{
  float depth  = /*TP_GLSL_TEXTURE_2D*/(depthSampler, texCoordinate).x;
  vec3  normal = normalize(/*TP_GLSL_TEXTURE_2D*/(normalsSampler, texCoordinate).xyz);

  // The fragment position in view space
  vec3 viewCoord = clipToView(texCoordinate, depth);

  vec3 cameraRay  = normalize(viewCoord);
  vec3 reflectRay = normalize(reflect(cameraRay, normal));

  for(float x=-1; x<=1.01; x+=0.5)
    for(float y=-1; y<=1.01; y+=0.5)
      for(float z=-1; z<=1.01; z+=0.5)
        takeSample(viewCoord, reflectRay, vec3(x,y,z)*0.1);

  if(count>1.0)
    fragColorR /= count;

  vec3 fragColorA = /*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate).xyz;
  float metalness = 0.01;///*TP_GLSL_TEXTURE_2D*/(specularSampler, texCoordinate).x;

  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(fragColorA+(fragColorR*metalness), 1.0);


  ///*TP_GLSL_GLFRAGCOLOR*/ = vec4(reflectRay, 1);


  ///*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE_2D*/(textureSampler, viewToClip(viewCoord + (reflectRay*0.1)).xy).xyz, 1.0);
  ///*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate).xyz, 1.0);

  ///*TP_GLSL_GLFRAGCOLOR*/ = vec4(0,0,1,1);

  ///*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/(normalsSampler, texCoordinate);
  // /*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_2D*/(textureSampler, texCoordinate);
  ///*TP_GLSL_GLFRAGCOLOR*/.x = 0.5;
}
