#include "tp_maps/shaders/Geometry3DShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{
namespace
{
//##################################################################################################
void drawVBO(Geometry3DShader::VertexBuffer* vertexBuffer)
{
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral( 0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral(12));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral(24));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glDisableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
}
}

//##################################################################################################
Geometry3DShader::Geometry3DShader(tp_maps::OpenGLProfile openGLProfile):
  Shader(openGLProfile)
{

}

//##################################################################################################
Geometry3DShader::VertexBuffer* Geometry3DShader::generateVertexBuffer(Map* map,
                                                                   const std::vector<GLuint>& indexes,
                                                                   const std::vector<Geometry3DShader::Vertex>& verts)const
{
  auto vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->vertexCount = GLuint(verts.size());
  vertexBuffer->indexCount  = TPGLsize(indexes.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLuint)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(Geometry3DShader::Vertex)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);
  drawVBO(vertexBuffer);
  tpBindVertexArray(0);
#endif

  return vertexBuffer;
}

//##################################################################################################
Geometry3DShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
Geometry3DShader::VertexBuffer::~VertexBuffer()
{
  if(!vaoID || !shader.shader())
    return;

  map->makeCurrent();
  tpDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &iboID);
  glDeleteBuffers(1, &vboID);
}

}
