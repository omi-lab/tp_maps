#include "tp_maps/shaders/FrameShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct FrameShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FrameShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  GLint scaleLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, FrameShader::VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_SHORT, nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
FrameShader::FrameShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Shader(map, openGLProfile),
  d(new Private())
{

}

//##################################################################################################
FrameShader::~FrameShader()
{
  delete d;
}

//##################################################################################################
void FrameShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void FrameShader::setScale(const glm::vec3& scale)
{
  glUniform3fv(d->scaleLocation, 1, &scale.x);
}

//##################################################################################################
void FrameShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
FrameShader::VertexBuffer* FrameShader::generateVertexBuffer(Map* map,
                                                             const std::vector<GLushort>& indexes,
                                                             const std::vector<FrameShader::Vertex>& verts) const
{
  auto vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->indexCount  = TPGLsizei(indexes.size());

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  vertexBuffer->vertexCount = GLuint(verts.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLushort)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(FrameShader::Vertex)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);
  vertexBuffer->bindVBO();
  tpBindVertexArray(0);
#else
  vertexBuffer->vertexCount = GLuint(indexes.size());

  std::vector<FrameShader::Vertex> indexedVerts;
  indexedVerts.reserve(indexes.size());
  for(auto index : indexes)
    indexedVerts.push_back(verts.at(size_t(index)));

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexedVerts.size()*sizeof(FrameShader::Vertex)), indexedVerts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

  return vertexBuffer;
}

//##################################################################################################
FrameShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
FrameShader::VertexBuffer::~VertexBuffer()
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
void FrameShader::VertexBuffer::bindVBO() const
{
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(FrameShader::Vertex), tpVoidLiteral( 0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(FrameShader::Vertex), tpVoidLiteral(12));
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(FrameShader::Vertex), tpVoidLiteral(24));
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(FrameShader::Vertex), tpVoidLiteral(36));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
#endif
}

//##################################################################################################
void FrameShader::draw(GLenum mode,
                            VertexBuffer* vertexBuffer,
                            const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void FrameShader::drawPicking(GLenum mode,
                                   VertexBuffer* vertexBuffer,
                                   const glm::vec4& pickingID)
{
  TP_UNUSED(pickingID);
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void FrameShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  Shader::use(shaderType);
}

//##################################################################################################
const char* FrameShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/FrameShader.vert"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
const char* FrameShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/FrameShader.frag"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
void FrameShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertexP");
  glBindAttribLocation(program, 1, "inVertexR");
  glBindAttribLocation(program, 2, "inNormal");
  glBindAttribLocation(program, 3, "inTexture");
}

//##################################################################################################
void FrameShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  d->matrixLocation = glGetUniformLocation(program, "matrix");
  d->scaleLocation  = glGetUniformLocation(program, "scale");
  d->colorLocation  = glGetUniformLocation(program, "color");

  if(d->matrixLocation<0)
    tpWarning() << "FrameShader d->matrixLocation: " << d->matrixLocation;

  if(d->scaleLocation<0)
    tpWarning()  << "FrameShader d->scaleLocation: "  << d->scaleLocation;

  if(d->colorLocation<0)
    tpWarning()  << "FrameShader d->colorLocation: "  << d->colorLocation;
}

//##################################################################################################
void FrameShader::init()
{
  if(map()->extendedFBO() == ExtendedFBO::Yes)
    compile(ShaderType::RenderExtendedFBO);
  else
    compile(ShaderType::Render);
}

}
