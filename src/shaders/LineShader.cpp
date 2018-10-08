#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
const char* vertexShaderStr =
    TP_VERT_SHADER_HEADER
    "uniform mat4 matrix;\n"
    TP_GLSL_IN_V "vec3 position;\n"
    "void main()\n"
    "{\n"
    "  gl_Position=matrix*vec4(position, 1.0);\n"
    "}";

const char* fragmentShaderStr =
    TP_FRAG_SHADER_HEADER
    "uniform vec3 color;\n"
    TP_GLSL_GLFRAGCOLOR_DEF
    "void main()\n"
    "{\n"
    "  " TP_GLSL_GLFRAGCOLOR "=vec4(color, 1.0);\n"
    "}";
}

//##################################################################################################
struct LineShader::Private
{
  GLint matrixLocation{0};
  //  GLint positionLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, LineShader::VertexBuffer* vertexBuffer)
  {
    if(vertexBuffer->indexCount<2)
      return;

    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    tpBindVertexArray(0);
  }
};

//##################################################################################################
LineShader::LineShader():
  Shader(),
  d(new Private())
{
  compile(vertexShaderStr,
          fragmentShaderStr,
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
void LineShader::setColor(const glm::vec3& color)
{
  glUniform3fv(d->colorLocation , 1, &color.x);
}

//##################################################################################################
LineShader::VertexBuffer* LineShader::generateVertexBuffer(Map* map, const std::vector<glm::vec3>& vertices)const
{
  VertexBuffer* vertexBuffer = new VertexBuffer(map, this);

  if(vertices.empty())
    return vertexBuffer;

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

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);

  tpBindVertexArray(0);

  return vertexBuffer;






//  LineShader::VertexBuffer* vertexBuffer = new LineShader::VertexBuffer(map);

//  glGenBuffers(1, &vertexBuffer->vboID);
//  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
//  glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);
//  vertexBuffer->vertexCount = vertices.size();

//  return vertexBuffer;
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
  if(!vaoID || !shader.shader())
    return;

  map->makeCurrent();
  tpDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &iboID);
  glDeleteBuffers(1, &vboID);
}

//##################################################################################################
void LineShader::drawLines(GLenum mode, LineShader::VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

}
