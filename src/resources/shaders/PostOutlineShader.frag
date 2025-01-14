#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D depthObjectSampler;
uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  if(TP_GLSL_TEXTURE_2D(depthObjectSampler, coord_tex).x>=1.0f)
  {
    for(int y=-2; y<3; y++)
    {
      float sy = coord_tex.y + pixelSize.y*float(y);
      if(sy<0.0f || sy>1.0f)
        continue;

      for(int x=-2; x<3; x++)
      {
        float sx = coord_tex.x + pixelSize.x*float(x);
        if(sx<0.0f || sx>1.0f)
          continue;

        if(TP_GLSL_TEXTURE_2D(depthObjectSampler, vec2(sx, sy)).x<1.0f)
        {
          TP_GLSL_GLFRAGCOLOR = vec4(0.25,0.25,0.8,1.0);
          return;
        }
      }
    }
  }

  TP_GLSL_GLFRAGCOLOR = TP_GLSL_TEXTURE_2D(textureSampler, coord_tex);
}
