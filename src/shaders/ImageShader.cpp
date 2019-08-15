#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderString vertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//ImageShader vertexShaderStr\n"
    "$TP_GLSL_IN_V$vec3 inVertex;\n"
    "$TP_GLSL_IN_V$vec3 inNormal;\n"
    "$TP_GLSL_IN_V$vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    "$TP_GLSL_OUT_V$vec3 LightVector0;\n"
    "$TP_GLSL_OUT_V$vec3 EyeNormal;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = matrix * vec4(inVertex, 1.0);\n"
    "  LightVector0 = vec3(1.0, 1.0, 1.0);\n"
    "  EyeNormal = inNormal;\n"
    "  texCoordinate = inTexture;\n"
    "}\n";

ShaderString fragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//ImageShader fragmentShaderStr\n"
    "$TP_GLSL_IN_F$vec3 LightVector0;\n"
    "$TP_GLSL_IN_F$vec3 EyeNormal;\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "uniform sampler2D textureSampler;\n"
    "uniform vec4 color;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = $TP_GLSL_TEXTURE$(textureSampler, texCoordinate)*color;\n"
    "  if($TP_GLSL_GLFRAGCOLOR$.a < 0.01)\n"
    "    discard;\n"
    "}\n";
}

//##################################################################################################
struct ImageShader::Private
{
  GLint matrixLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, ImageShader::VertexBuffer* vertexBuffer)
  {
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_INT,
                   nullptr);
    tpBindVertexArray(0);

  }
};

//##################################################################################################
ImageShader::ImageShader(tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  Geometry3DShader(openGLProfile),
  d(new Private())
{
  if(!vertexShader)
    vertexShader = vertexShaderStr.data(openGLProfile);

  if(!fragmentShader)
    fragmentShader = fragmentShaderStr.data(openGLProfile);

  compile(vertexShader,
          fragmentShader,
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
    glBindAttribLocation(program, 2, "inTexture");
  },
  [this](GLuint program)
  {
    d->matrixLocation = glGetUniformLocation(program, "matrix");
    d->colorLocation  = glGetUniformLocation(program, "color");
    const char* shaderName = "ImageShader";
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->matrixLocation: " << d->matrixLocation;
  });
}

//##################################################################################################
ImageShader::~ImageShader()
{
  delete d;
}

//##################################################################################################
void ImageShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  Shader::use(shaderType);
}

//##################################################################################################
void ImageShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void ImageShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
void ImageShader::draw(GLenum mode,
                            VertexBuffer* vertexBuffer,
                            const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void ImageShader::drawPicking(GLenum mode,
                                   VertexBuffer* vertexBuffer,
                                   const glm::vec4& pickingID)
{
  TP_UNUSED(pickingID);
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}

}
