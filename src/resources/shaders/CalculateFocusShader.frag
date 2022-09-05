/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;
uniform sampler2D normalsSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;

float near = 0.01;
float far  = 100.0;

/*DOF_FRAG_VARS*/

/*TP_GLSL_GLFRAGCOLOR_DEF*/

float LinearizeDepth( float depth )
{
  float z = depth * 2.0 - 1.0; // back to NDC
  return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
  vec3 color = vec3(0.0,0.0,0.0);

  float f = 0.0f;

  float depth = /*TP_GLSL_TEXTURE_2D*/(depthSampler, coord_tex).x;
  depth = LinearizeDepth(depth);

  if( depth < focalDistance )
  {
    f = (depth - focalDistance) / (focalDistance - nearPlane);
  }
  else
  {
    f = (depth - focalDistance) / (farPlane - focalDistance);
    // Clamp to maximum bluriness
    f = clamp(f, 0, blurinessCutoffConstant);
  }

  // Scale and bias into [0,1] range
  float result = f * 0.5f + 0.5f;

   /*TP_GLSL_GLFRAGCOLOR*/ = vec4( result, 0.0, 0.0, 1.0 );
}
