#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
  vec3 rgb = TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).xyz;
  float sdiv = 0.0625f;
  float div1 = 1260.0f/3306.0f;
  float div2 = 1760.0f/3306.0f;
  float div3 = 2081.0f/3306.0f;
  float div4 = 2310.0f/3306.0f;
  float div5 = 2488.0f/3306.0f;
  float div6 = 2630.0f/3306.0f;
  float div7 = 2747.0f/3306.0f;
  float div8 = 2846.0f/3306.0f;
  float div9 = 2931.0f/3306.0f;
  float div10 = 3005.0f/3306.0f;
  float div11 = 3069.0f/3306.0f;
  float div12 = 3127.0f/3306.0f;
  float div13 = 3179.0f/3306.0f;
  float div14 = 3225.0f/3306.0f;
  float div15 = 3267.0f/3306.0f;
  for(int c=0; c<3; ++c)
  {
    if(rgb[c] < sdiv)
      rgb[c] = mix(0.f, div1, (rgb[c] - 0.f)/sdiv);
    else if(rgb[c] < 2.0f*sdiv)
      rgb[c] = mix(div1, div2, (rgb[c] - sdiv)/sdiv);
    else if(rgb[c] < 3.0f*sdiv)
      rgb[c] = mix(div2, div3, (rgb[c] - 2.0f*sdiv)/sdiv);
    else if(rgb[c] < 4.0f*sdiv)
      rgb[c] = mix(div3, div4, (rgb[c] - 3.0f*sdiv)/sdiv);
    else if(rgb[c] < 5.0f*sdiv)
      rgb[c] = mix(div4, div5, (rgb[c] - 4.0f*sdiv)/sdiv);
    else if(rgb[c] < 6.0f*sdiv)
      rgb[c] = mix(div5, div6, (rgb[c] - 5.0f*sdiv)/sdiv);
    else if(rgb[c] < 7.0f*sdiv)
      rgb[c] = mix(div6, div7, (rgb[c] - 6.0f*sdiv)/sdiv);
    else if(rgb[c] < 8.0f*sdiv)
      rgb[c] = mix(div7, div8, (rgb[c] - 7.0f*sdiv)/sdiv);
    else if(rgb[c] < 9.0f*sdiv)
      rgb[c] = mix(div8, div9, (rgb[c] - 8.0f*sdiv)/sdiv);
    else if(rgb[c] < 10.0f*sdiv)
      rgb[c] = mix(div9, div10, (rgb[c] - 9.0f*sdiv)/sdiv);
    else if(rgb[c] < 11.0f*sdiv)
      rgb[c] = mix(div10, div11, (rgb[c] - 10.0f*sdiv)/sdiv);
    else if(rgb[c] < 12.0f*sdiv)
      rgb[c] = mix(div11, div12, (rgb[c] - 11.0f*sdiv)/sdiv);
    else if(rgb[c] < 13.0f*sdiv)
      rgb[c] = mix(div12, div13, (rgb[c] - 12.0f*sdiv)/sdiv);
    else if(rgb[c] < 14.0f*sdiv)
      rgb[c] = mix(div13, div14, (rgb[c] - 13.0f*sdiv)/sdiv);
    else if(rgb[c] < 15.0f*sdiv)
      rgb[c] = mix(div14, div15, (rgb[c] - 14.0f*sdiv)/sdiv);
    else
      rgb[c] = mix(div15, 1.0f, (rgb[c] - 15.0f*sdiv)/sdiv);
  }

  //Note: GammaCorrection
  rgb *= 0.1;
  TP_GLSL_GLFRAGCOLOR = vec4(rgb, 1.0);
  //TP_GLSL_GLFRAGCOLOR = vec4(pow(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).xyz, vec3(1.0f/2.2)), 1.0);

  //Note: View transform
  //vec3 hsv = rgb2hsv(TP_GLSL_TEXTURE_2D(textureSampler, coord_tex).xyz);
  //hsv.y *= 0.85f;
  //hsv.z *= 0.1f;
  //TP_GLSL_GLFRAGCOLOR = vec4(hsv2rgb(hsv), 1.0f); //vec4(0,0,0,1);
}
