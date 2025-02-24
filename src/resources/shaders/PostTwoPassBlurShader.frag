#pragma replace TP_FRAG_SHADER_HEADER
#define TP_GLSL_IN_F
#define TP_GLSL_GLFRAGCOLOR
#define TP_GLSL_TEXTURE_2D

TP_GLSL_IN_F vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

uniform vec2 pixelSize;

#pragma replace TP_GLSL_GLFRAGCOLOR_DEF

uniform int dirIndex;
uniform float blurRadius;
uniform float blurSigma;

float gauss(float x, float sigma)
{
	return  1.0 / (3.14159 * sigma) * exp(-(x * x) / sigma);
}

void main()
{
  vec2 f = vec2(0.0);  // => Blur direction / Size
  f[dirIndex] = pixelSize[dirIndex];

	float totalWeight = 0.0;
	vec4  color       = vec4(0.0);

  float radiusHalf  = float(int(blurRadius * 0.5));
	for(float step = -radiusHalf; step <= radiusHalf; step += 2.0)
	{
		vec2 off_uv = coord_tex + vec2(step + 0.5) * f;
		//off_uv.x += (step + 0.5) * f.x;
		//off_uv.y += (step + 0.5) * f.y;

		float weight = gauss(step, blurSigma) + gauss(step + 1.0, blurSigma);
		color       += texture(textureSampler, off_uv) * weight;
		totalWeight += weight;
	}

  TP_GLSL_GLFRAGCOLOR = color * (1.0 / totalWeight);
}
