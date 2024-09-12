
//##################################################################################################
vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

//##################################################################################################
vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

//##################################################################################################
vec3 toLinear(vec3 color)
{
  vec3 hsv = rgb2hsv(pow(color, vec3(2.2)));
  hsv.y /= 0.9;
  return hsv2rgb(hsv);
}

//##################################################################################################
vec4 toLinear(vec4 color)
{
  return vec4(toLinear(color.xyz), color.w);
}

//##################################################################################################
vec3 fromLinear(vec3 color)
{
  vec3 hsv = rgb2hsv(color);
  hsv.y *= 0.9;
  return pow(hsv2rgb(hsv), vec3(1.0/2.2));
}

//##################################################################################################
vec4 fromLinear(vec4 color)
{
  return vec4(fromLinear(color.xyz), color.w);
}
