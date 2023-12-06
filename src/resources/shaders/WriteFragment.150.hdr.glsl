//out vec4 fragNormal;
//out vec4 fragSpecular;

void writeFragment(vec3 ambient, vec3 diffuse, vec3 specular, float alpha)
{
  if(alpha<discardOpacity)
    discard;

  vec3 result = ambient + diffuse + specular;

  fragColor = vec4(result, alpha);
  //fragNormal = vec4(normal, 1.0);
  //fragSpecular = vec4(shininess, shininess, shininess, 1.0);
}
