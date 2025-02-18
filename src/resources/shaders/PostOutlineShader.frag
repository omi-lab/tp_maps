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

uniform sampler2D outlineSampler;
uniform int mode;  // 0: calculate outline, 1: merge 'outlineSampler' with 'textureSampler'

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

//--- HELPERS ---------------------------------------------------

float objDepthAt(in vec2 uv) { return TP_GLSL_TEXTURE_2D(depthObjectSampler, uv).x; }
float objDepthAt(in float x, in float y) { return objDepthAt(vec2(x,y)); }

//--- OUTLINE ---------------------------------------------------

float calculateOutline()
{
  if (objDepthAt(coord_tex) < 1.0)
    return 0.0;

  for (float y = -2.0; y <= 2.0; ++y)
  {
    float sy = coord_tex.y + pixelSize.y * y;
    if (sy < 0.0 || sy > 1.0)
      continue;

    for(float x = -2.0; x <= 2.0; ++x)
    {
      float sx = coord_tex.x + pixelSize.x * x;
      if(sx < 0.0 || sx > 1.0)
        continue;

      float d = objDepthAt(sx, sy);
      if(d < 1.0)
        return d;
    }
  }

  return 0.0;
}

//--- OUTLINE ---------------------------------------------------

// vec4[9] make_kernel(sampler2D tex, vec2 coord)
// {
//   vec4 n[9];
// 	float w = pixelSize.x;
// 	float h = pixelSize.y;

// 	n[0] = texture( tex, coord + vec2( -w, -h ) );
// 	n[1] = texture( tex, coord + vec2(0.0, -h ) );
// 	n[2] = texture( tex, coord + vec2(  w, -h ) );
// 	n[3] = texture( tex, coord + vec2( -w, 0.0) );
// 	n[4] = texture( tex, coord );
// 	n[5] = texture( tex, coord + vec2(  w, 0.0) );
// 	n[6] = texture( tex, coord + vec2( -w,  h ) );
// 	n[7] = texture( tex, coord + vec2(0.0,  h ) );
// 	n[8] = texture( tex, coord + vec2(  w,  h ) );

//   return n;
// }

// vec4 calculateOutline()
// {
//   vec4 n[9] = make_kernel(depthObjectSampler, coord_tex);

// 	vec4 sobel_edge_h = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);
//   vec4 sobel_edge_v = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);
// 	vec4 sobel = sqrt((sobel_edge_h * sobel_edge_h) + (sobel_edge_v * sobel_edge_v));

//   return vec4(sobel.rgb, 1.0);
// 	// return vec4( 1.0 - sobel.rgb, 1.0 ).r;
// }

//--- ENTRY POINT -----------------------------------------------

void main()
{
  if (mode == 0)
  {
    // TP_GLSL_GLFRAGCOLOR = calculateOutline();
    TP_GLSL_GLFRAGCOLOR = vec4(vec3(calculateOutline()), 1.0);
    return;
  }

  vec4 outlineColor = vec4(0.4,0.4,0.95,1.0);
  vec4 mask = TP_GLSL_TEXTURE_2D(outlineSampler, coord_tex);
  vec4 srcColor = TP_GLSL_TEXTURE_2D(textureSampler, coord_tex);

  TP_GLSL_GLFRAGCOLOR = mix(srcColor, outlineColor, mask.r);
}
