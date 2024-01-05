#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;


// ------------------------------------------------
uniform sampler2D downsampledTextureSampler;
uniform sampler2D focusTextureSampler;
uniform sampler2D downsampledFocusTextureSampler;

vec2 poisson[6];

vec2 vMaxCoC = vec2(5.0, 10.0);
float radiusScale = 0.4;

int numTaps = 6;

#pragma replace DOF_FRAG_VARS
#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

void main()
{
  // Set poisson kernel
  poisson[0] = vec2( 0.000000, 1.000000 );
  poisson[1] = vec2( 0.000000, -1.000000 );
  poisson[2] = vec2( -0.866025, 0.500000 );
  poisson[3] = vec2( -0.866025, -0.500000 );
  poisson[4] = vec2( 0.866025, 0.500000 );
  poisson[5] = vec2( 0.866025, -0.500000 );

  vec2 pixelSizeHigh = pixelSize;
  vec2 pixelSizeLow = pixelSize * 4.0;

  vec4 color;

  float discRadius, discRadiusLow, centerDepth;

  color = TP_GLSL_TEXTURE_2D(textureSampler, coord_tex);

  float focus = TP_GLSL_TEXTURE_2D(focusTextureSampler, coord_tex).r;
  centerDepth = focus;

  // Convert the depth value in alpha to a blur radius in pixels
  discRadius = abs( focus * vMaxCoC.y - vMaxCoC.x );

  discRadiusLow = discRadius * radiusScale;

  color = vec4(0,0,0,0);

  for( int t = 0; t < numTaps; t++ )
  {
    vec2 coordLow = coord_tex + (pixelSizeLow * poisson[t] * discRadiusLow);
    vec2 coordHigh = coord_tex + (pixelSizeHigh * poisson[t] * discRadius);

    // Mix low and high res textures based on tap bluriness
    // This gets the colors
    vec4 tapLow = TP_GLSL_TEXTURE_2D(downsampledTextureSampler, coordLow);
    vec4 tapHigh = TP_GLSL_TEXTURE_2D(textureSampler, coordHigh);

    // This gets the depth values
    float depthTapLow = TP_GLSL_TEXTURE_2D(downsampledFocusTextureSampler, coordLow).r;
    float depthTapHigh = TP_GLSL_TEXTURE_2D(focusTextureSampler, coordHigh).r;

    float tapBlur = abs( depthTapHigh * 2.0 - 1.0 );
    vec4 tap = mix( tapHigh, tapLow, tapBlur );

    float tapDepth = mix( depthTapHigh, depthTapLow, tapBlur );

    // 'Smart' blur ignores taps that are closer than the center tap and in focus
    tapDepth = ( tapDepth >= centerDepth ) ? 1.0 : abs( tapDepth * 2.0 - 1.0 );

    color.rgb += tap.rgb * tapDepth;
    color.a += tapDepth;
  }

   TP_GLSL_GLFRAGCOLOR = (color / color.a);
}
