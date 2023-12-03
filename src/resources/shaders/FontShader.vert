/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;
/*TP_GLSL_IN_V*/vec4 inNormal;
/*TP_GLSL_IN_V*/vec4 inTangent;
/*TP_GLSL_IN_V*/vec2 inTexture;

uniform mat4 matrix;

/*TP_GLSL_OUT_V*/vec2 coord_tex;

void main()
{
  gl_Position = matrix * vec4(inVertex, 1.0);
  coord_tex = inTexture;
}
