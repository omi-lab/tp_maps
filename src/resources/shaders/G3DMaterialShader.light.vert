/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec3 inVertex;
/*TP_GLSL_IN_V*/vec2 inTexture;

/*TP_GLSL_OUT_V*/vec2 uv_tangent;

uniform mat4 mvp;
uniform mat3 uvMatrix;

void main()
{
  gl_Position = mvp * vec4(inVertex, 1.0);

  vec3 uv = uvMatrix * vec3(inTexture, 1.0f);
  uv_tangent = uv.xy;// / uv.z;
}
