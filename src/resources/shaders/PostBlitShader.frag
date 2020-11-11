/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 texCoordinate;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE*/(textureSampler, texCoordinate).xyz, 1.0);
}
