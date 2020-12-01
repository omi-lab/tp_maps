
void writeFragment(vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, float alpha, vec3 materialSpecular, float shininess)
{
  if(alpha<discardOpacity)
    discard;

  vec3 result = ambient + diffuse + specular;

  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(result, alpha);
}
