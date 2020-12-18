/*TP_VERT_SHADER_HEADER*/

/*TP_GLSL_IN_V*/vec4 inColor;
/*TP_GLSL_IN_V*/vec3 inPosition;
/*TP_GLSL_IN_V*/vec3 inOffset;
/*TP_GLSL_IN_V*/vec2 inTexture;

uniform mat4 matrix;
uniform vec2 scaleFactor;

/*TP_GLSL_OUT_V*/vec2 coord_tex;
/*TP_GLSL_OUT_V*/vec4 color;
/*TP_GLSL_OUT_V*/float clip;

void main()
{
  gl_Position = (matrix * vec4(inPosition, 1.0));
  clip = (gl_Position.z<-0.9999)?0.0:1.0;
  gl_Position += vec4((inOffset.x*scaleFactor.x)*gl_Position.w, (inOffset.y*scaleFactor.y)*gl_Position.w, inOffset.z, 0.0);
  coord_tex = inTexture;
  color = inColor;
}
