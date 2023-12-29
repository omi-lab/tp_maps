#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

//uniform vec2 pixelSize;

const vec3 gridColor = vec3(0.05, 0.05, 0.9); // blue
const vec2 gridCentre = vec2(0.5f, 0.5f); // in texture coordinates
const float gridSpacing = 0.1f;
const float lineHalfWidth = 0.001f;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

bool isOnLine(float value)
{
  return (value >= (gridSpacing - lineHalfWidth) || value <= lineHalfWidth);
}

void main()
{
  vec3 color;

  //float sx = coord_tex.x + pixelSize.x*float(x);
  //float sy = coord_tex.y + pixelSize.y*float(y);

  vec2 offsetToGridCentre = abs(coord_tex - gridCentre);

  if (offsetToGridCentre.x <= lineHalfWidth
    || offsetToGridCentre.y <= lineHalfWidth)
    // Pixel on the central line
    color = vec3(1.0, 0.0, 0.0); // red
  else
  {
    // Test if pixel is on a vertical line
    float distanceToClosestLine = mod(offsetToGridCentre.x, gridSpacing);
    if (isOnLine(distanceToClosestLine))
      color = gridColor;
    else
    {
      // Test if pixel is on a horizontal line
      distanceToClosestLine = mod(offsetToGridCentre.y, gridSpacing);
      if (isOnLine(distanceToClosestLine))
        color = gridColor;
      else
        // Normal pixel
        discard;
    }
  }

  TP_GLSL_GLFRAGCOLOR = vec4(color, 1.0);
}
