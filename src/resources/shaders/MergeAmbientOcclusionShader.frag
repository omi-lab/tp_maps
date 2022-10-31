/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;
uniform sampler2D normalsSampler;
uniform sampler2D ssaoTextureSampler;

const float discardOpacity=0.8;

/*AO_FRAG_VARS*/

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

void main()
{
  float occlusion = /*TP_GLSL_TEXTURE_2D*/(ssaoTextureSampler, coord_tex).x;

  if( occlusion < boostUpperThreshold ) {
    occlusion *= boostUpperFactor;
  }
  else if ( occlusion < boostLowerThreshold ) {
    occlusion *= boostLowerFactor;
  }

  vec3 ambient = /*TP_GLSL_TEXTURE_2D*/(textureSampler, coord_tex).xyz * occlusion;

  ambient = pow(ambient, vec3(1.0/1.2));

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  vec3 normal = normalize(/*TP_GLSL_TEXTURE_2D*/(normalsSampler , coord_tex).xyz);
  float alpha = 1.0;
  vec3 materialSpecular = vec3(0.0);
  float shininess = 0.0;

  writeFragment(ambient, diffuse, specular, normal, alpha, materialSpecular, shininess);
}
