//varying vec4 fragNormal;
//varying vec4 fragSpecular;

void writeFragment(vec3 ambient, vec3 diffuse, vec3 specular, float alpha)
{
  if(alpha<discardOpacity)
    discard;

  vec3 result = ambient + diffuse + specular;

  gl_FragColor = vec4(result, alpha);
}
