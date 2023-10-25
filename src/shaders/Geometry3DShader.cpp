#include "tp_maps/shaders/Geometry3DShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
Geometry3DShader::VertexBuffer* Geometry3DShader::generateVertexBuffer(Map* map,
                                                                       const std::vector<GLuint>& indexes,
                                                                       const std::vector<Geometry3DShader::Vertex>& verts) const
{
  auto vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->indexCount  = TPGLsizei(indexes.size());

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  vertexBuffer->vertexCount = GLuint(verts.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLuint)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(Geometry3DShader::Vertex)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);
  vertexBuffer->bindVBO();
  tpBindVertexArray(0);
#else
  vertexBuffer->vertexCount = GLuint(indexes.size());
  std::vector<Geometry3DShader::Vertex> indexedVerts;
  indexedVerts.reserve(indexes.size());
  for(auto index : indexes)
    indexedVerts.push_back(verts.at(size_t(index)));

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexedVerts.size()*sizeof(Geometry3DShader::Vertex)), indexedVerts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

  return vertexBuffer;
}

//##################################################################################################
void Geometry3DShader::init()
{
  if(map()->extendedFBO() == ExtendedFBO::Yes)
    compile(ShaderType::RenderExtendedFBO);
  else
    compile(ShaderType::Render);

#ifdef TP_GLSL_PICKING_SUPPORTED
  compile(ShaderType::Picking);
#endif

  compile(ShaderType::Light);
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
void Geometry3DShader::VertexBuffer::bindVBO() const
{
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral( 0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral(12));
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Geometry3DShader::Vertex), tpVoidLiteral(24));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
#endif
}

}
