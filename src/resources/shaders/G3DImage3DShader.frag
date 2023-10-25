/*TP_FRAG_SHADER_HEADER*/

/*TP_GLSL_IN_F*/vec3 LightVector0;
/*TP_GLSL_IN_F*/vec3 EyeNormal;
/*TP_GLSL_IN_F*/vec2 coord_tex;

uniform sampler3D textureSampler;
uniform vec4 color;

/*TP_GLSL_GLFRAGCOLOR_DEF*/

void main()
{
  /*TP_GLSL_GLFRAGCOLOR*/ = /*TP_GLSL_TEXTURE_3D*/(textureSampler, vec3(coord_tex, 0))*color;
  if(/*TP_GLSL_GLFRAGCOLOR*/.a < 0.01)
    discard;
}
