/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  //Note: GammaCorrection
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(pow(/*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz, vec3(1.0f/2.2)), 1.0);
}
