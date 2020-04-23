#include "tp_maps/shaders/FrameShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderString vertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//FrameShader vertexShaderStr\n"
    "$TP_GLSL_IN_V$vec3 inVertexP;\n"
    "$TP_GLSL_IN_V$vec3 inVertexR;\n"
    "$TP_GLSL_IN_V$vec3 inNormal;\n"
    "$TP_GLSL_IN_V$vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    "uniform vec3 scale;\n"
    "uniform vec4 color;\n"
    "$TP_GLSL_OUT_V$vec3 lightVector0;\n"
    "$TP_GLSL_OUT_V$vec3 eyeNormal;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec4 multColor;\n"
    "void main()\n"
    "{\n"
    "  vec3 inVertex = inVertexP+(inVertexR*scale);\n"
    "  gl_Position = matrix * vec4(inVertex, 1.0);\n"
    "  lightVector0 = vec3(1.0, 1.0, 1.0);\n"
    "  eyeNormal = inNormal;\n"
    "  texCoordinate = inTexture;\n"
    "  multColor = color;\n"
    "}\n";

ShaderString fragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//FrameShader fragmentShaderStr\n"
    "$TP_GLSL_IN_F$vec3 lightVector0;\n"
    "$TP_GLSL_IN_F$vec3 eyeNormal;\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "$TP_GLSL_IN_F$vec4 multColor;\n"
    "uniform sampler2D textureSampler;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = $TP_GLSL_TEXTURE$(textureSampler, texCoordinate)*multColor;\n"
    "  if($TP_GLSL_GLFRAGCOLOR$.a < 0.01)\n"
    "    discard;\n"
    "}\n";
}

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
FrameShader::FrameShader(tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  Shader(openGLProfile),
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
    glBindAttribLocation(program, 0, "inVertexP");
    glBindAttribLocation(program, 1, "inVertexR");
    glBindAttribLocation(program, 2, "inNormal");
    glBindAttribLocation(program, 3, "inTexture");
  },
  [this](GLuint program)
  {
    d->matrixLocation = glGetUniformLocation(program, "matrix");
    d->scaleLocation  = glGetUniformLocation(program, "scale");
    d->colorLocation  = glGetUniformLocation(program, "color");
    const char* shaderName = "FrameShader";
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->matrixLocation: " << d->matrixLocation;
    if(d->scaleLocation<0)tpWarning()  << shaderName << " d->scaleLocation: "  << d->scaleLocation;
    if(d->colorLocation<0)tpWarning()  << shaderName << " d->colorLocation: "  << d->colorLocation;
  });
}

//##################################################################################################
FrameShader::~FrameShader()
{
  delete d;
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
                                                             const std::vector<FrameShader::Vertex>& verts)const
{
  auto vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->indexCount  = TPGLsize(indexes.size());

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

}
