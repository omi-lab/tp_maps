$TP_FRAG_SHADER_HEADER$

$TP_GLSL_IN_F$vec3 LightVector0;
$TP_GLSL_IN_F$vec3 EyeNormal;
$TP_GLSL_IN_F$vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform vec4 color;

$TP_GLSL_GLFRAGCOLOR_DEF$

void main()
{
  float near = 0.01;
  float far  = 100.0;

  // Get the z value from the depth texture.
  float depth = $TP_GLSL_TEXTURE$(textureSampler, texCoordinate).x;

  // Scale the depth back into world coords.
  depth = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

  // Scale again into the depth range that we are interested in rendering.
  //depth = depth / 0.1;

  $TP_GLSL_GLFRAGCOLOR$ = vec4(depth,depth,depth,1.0);
}
