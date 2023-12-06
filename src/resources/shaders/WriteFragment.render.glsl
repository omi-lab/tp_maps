
void writeFragment(vec3 ambient, vec3 diffuse, vec3 specular, float alpha)
{
  if(alpha<discardOpacity)
    discard;

  vec3 result = ambient + diffuse + specular;

  /*TP_GLSL_GLFRAGCOLOR*/ = vec4(result, alpha);
}
