#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/LineShader.vert"}; return s;}
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/LineShader.frag"}; return s;}
}

//##################################################################################################
struct LineShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::LineShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  //  GLint positionLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, LineShader::VertexBuffer* vertexBuffer)
  {
    if(vertexBuffer->indexCount<2)
      return;

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
LineShader::LineShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Shader(map, openGLProfile),
  d(new Private())
{
  compile(vertShaderStr().data(openGLProfile, ShaderType::Render),
          fragShaderStr().data(openGLProfile, ShaderType::Render),
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "position");
  },
  [this](GLuint program)
  {
    d->matrixLocation   = glGetUniformLocation(program, "matrix");
    d->colorLocation    = glGetUniformLocation(program, "color");
  });
}

//##################################################################################################
LineShader::~LineShader()
{
  delete d;
}

//##################################################################################################
void LineShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html

  switch(shaderType)
  {
  case ShaderType::Light: [[fallthrough]];
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderHDR:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    break;

  case ShaderType::Picking:
    glDisable(GL_BLEND);
    break;
  }

  Shader::use(ShaderType::Render);
}

//##################################################################################################
void LineShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void LineShader::setLineWidth(float lineWidth)
{
  glLineWidth(lineWidth);
}

//##################################################################################################
void LineShader::setColor(const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
}

//##################################################################################################
LineShader::VertexBuffer* LineShader::generateVertexBuffer(Map* map, const std::vector<glm::vec3>& vertices) const
{
  auto vertexBuffer = new VertexBuffer(map, this);

  if(vertices.empty())
    return vertexBuffer;

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  std::vector<GLuint> indexes;
  indexes.reserve(vertices.size());
  for(GLuint i=0; i<GLuint(vertices.size()); i++)
    indexes.push_back(i);

  vertexBuffer->vertexCount = GLuint(vertices.size());
  vertexBuffer->indexCount  = GLuint(indexes.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLuint)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vertices.size()*sizeof(glm::vec3)), vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);
  vertexBuffer->bindVBO();
  tpBindVertexArray(0);
#else
  vertexBuffer->vertexCount = GLuint(vertices.size());
  vertexBuffer->indexCount  = GLuint(vertices.size());

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vertices.size()*sizeof(glm::vec3)), vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

  return vertexBuffer;
}

//##################################################################################################
LineShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
LineShader::VertexBuffer::~VertexBuffer()
{
  if(!shader.shader())
    return;

  map->makeCurrent();

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  if(vaoID)
    tpDeleteVertexArrays(1, &vaoID);

  if(iboID)
    glDeleteBuffers(1, &iboID);
#endif

  if(vboID)
    glDeleteBuffers(1, &vboID);
}

//##################################################################################################
void LineShader::VertexBuffer::bindVBO() const
{
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
#endif
}

//##################################################################################################
void LineShader::drawLines(GLenum mode, LineShader::VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

}
