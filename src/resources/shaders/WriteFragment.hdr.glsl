layout(location = 1) out vec4 fragNormal;
layout(location = 2) out vec4 fragSpecular;

void writeFragment(vec3 ambient, vec3 diffuse, vec3 specular, vec3 normal, float alpha, vec3 materialSpecular, float shininess)
{
  if(alpha<discardOpacity)
    discard;

  vec3 result = ambient + diffuse + specular;

  fragColor = vec4(result, alpha);
  fragNormal = vec4(normal, 1.0);
  fragSpecular = vec4(shininess, shininess, shininess, 1.0);
}
