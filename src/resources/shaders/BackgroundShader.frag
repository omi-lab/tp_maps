/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler2D textureSampler;

const float discardOpacity=0.8;

uniform mat4 vpInv;

uniform float rotationFactor;

/*TP_GLSL_GLFRAGCOLOR_DEF*/
/*TP_WRITE_FRAGMENT*/

//##################################################################################################
void main()
{
  vec4 v = vec4(coord_tex*2.0 - 1.0, 0.0, 1.0);

  v.z=0.0;
  vec4 near = vpInv * v;

  v.z=1.0;
  vec4 far = vpInv * v;

  vec3 d = normalize((far.xyz / far.w) - (near.xyz / near.w));

  vec2 textureCoord;
  textureCoord.x = 1.0 - (atan(d.y, d.x) / (2.0*3.1415926538) + 0.5);
  textureCoord.y = acos(d.z) / 3.1415926538;

  textureCoord.x += rotationFactor;

  vec3 ambient = /*TP_GLSL_TEXTURE_2D*/(textureSampler, textureCoord).xyz;

  //Note: GammaCorrection
  ambient = pow(ambient, vec3(2.2));

  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);
  vec3 normal = vec3(0.0,0.0,1.0);
  float alpha = 1.0;
  vec3 materialSpecular = vec3(0.0);
  float shininess = 0.0;

  writeFragment(ambient, diffuse, specular, normal, alpha, materialSpecular, shininess);
}
