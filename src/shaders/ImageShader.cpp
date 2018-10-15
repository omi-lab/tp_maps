#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

const char* vertexShaderStr =
    TP_VERT_SHADER_HEADER
    TP_GLSL_IN_V"vec3 inVertex;\n"
    TP_GLSL_IN_V"vec3 inNormal;\n"
    TP_GLSL_IN_V"vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    TP_GLSL_OUT_V"vec3 LightVector0;\n"
    TP_GLSL_OUT_V"vec3 EyeNormal;\n"
    TP_GLSL_OUT_V"vec2 texCoordinate;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = matrix * vec4(inVertex, 1.0);\n"
    "  LightVector0 = vec3(1.0, 1.0, 1.0);\n"
    "  EyeNormal = inNormal;\n"
    "  texCoordinate = inTexture;\n"
    "}\n";

const char* fragmentShaderStr =
    TP_FRAG_SHADER_HEADER
    TP_GLSL_IN_F"vec3 LightVector0;\n"
    TP_GLSL_IN_F"vec3 EyeNormal;\n"
    TP_GLSL_IN_F"vec2 texCoordinate;\n"
    "uniform sampler2D textureSampler;\n"
    "uniform vec4 color;\n"
    TP_GLSL_GLFRAGCOLOR_DEF
    "void main()\n"
    "{\n"
    "  " TP_GLSL_GLFRAGCOLOR " = " TP_GLSL_TEXTURE "(textureSampler, texCoordinate)*color;\n"
    "  if(" TP_GLSL_GLFRAGCOLOR ".a < 0.01)\n"
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
                        GL_UNSIGNED_SHORT,
                        nullptr);
    tpBindVertexArray(0);
  }
};

//##################################################################################################
ImageShader::ImageShader(const char* vertexShader, const char* fragmentShader):
  Shader(),
  d(new Private())
{
  if(!vertexShader)
    vertexShader = vertexShaderStr;

  if(!fragmentShader)
    fragmentShader = fragmentShaderStr;

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
ImageShader::VertexBuffer* ImageShader::generateVertexBuffer(Map* map,
                                                             const std::vector<GLushort>& indexes,
                                                             const std::vector<ImageShader::Vertex>& verts)const
{
  VertexBuffer* vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->vertexCount = GLuint(verts.size());
  vertexBuffer->indexCount = GLsizei(indexes.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLushort)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(ImageShader::Vertex)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ImageShader::Vertex), reinterpret_cast<void*>(0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ImageShader::Vertex), reinterpret_cast<void*>(sizeof(float)*3));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ImageShader::Vertex), reinterpret_cast<void*>(sizeof(float)*6));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glDisableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);

  tpBindVertexArray(0);

  return vertexBuffer;
}

//##################################################################################################
ImageShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
ImageShader::VertexBuffer::~VertexBuffer()
{
  if(!vaoID)
    return;

  map->makeCurrent();
  tpDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &iboID);
  glDeleteBuffers(1, &vboID);
}

//##################################################################################################
void ImageShader::drawImage(GLenum mode,
                            VertexBuffer* vertexBuffer,
                            const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, reinterpret_cast<const GLfloat*>(&color));
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void ImageShader::drawImagePicking(GLenum mode,
                                   VertexBuffer* vertexBuffer,
                                   const glm::vec4& pickingID)
{
  TP_UNUSED(pickingID);
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}

}
