/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D depthSampler;

uniform mat4 projectionMatrix;
uniform mat4 invProjectionMatrix;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{


  // //Note: GammaCorrection
  // auto convert = [&](float c)
  // {
  //   c *= scale;                  // Divide by n samples.
  //   c = std::pow(c, 1.0f/2.2f);  // Gamma correction.
  //   c = std::log10(c*6.0f+1.0f); // Adjust brightness & contrast.
  //
  //   return uint8_t(std::clamp(int(c*255.0f), 0, 255));
  // };
  // return{convert(in.x), convert(in.y), convert(in.z), 255};



  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(pow(/*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz, vec3(1.0f/2.2)), 1.0);
  ///*TP_GLSL_GLFRAGCOLOR*/ = vec4(/*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz, 1.0);
  ///*TP_GLSL_GLFRAGCOLOR*/.x=1.0;
}
