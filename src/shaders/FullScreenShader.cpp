#include "tp_maps/shaders/FullScreenShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/FullScreenShader.vert"}; return s;}
}

//##################################################################################################
struct FullScreenShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FullScreenShader::Private");
  TP_NONCOPYABLE(Private);

  GLint  frameMatrixLocation{0};

  std::unique_ptr<Object> object;

  Private()=default;
};

//##################################################################################################
FullScreenShader::FullScreenShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Shader(map, openGLProfile),
  d(new Private)
{  
  d->object.reset(makeRectangleObject({1.0f, 1.0f}));
}

//##################################################################################################
FullScreenShader::~FullScreenShader()
{
  delete d;
}

//##################################################################################################
void FullScreenShader::compile(const char* vertexShader,
                               const char* fragmentShader,
                               const std::function<void(GLuint)>& bindLocations,
                               const std::function<void(GLuint)>& getLocations,
                               ShaderType shaderType)
{
  if(!vertexShader)
    vertexShader = vertShaderStr().data(openGLProfile(), shaderType);

  Shader::compile(vertexShader, fragmentShader, [&](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");

    if(bindLocations)
      bindLocations(program);
  }, [&](GLuint program)
  {
    d->frameMatrixLocation = glGetUniformLocation(program, "frameMatrix");

    if(getLocations)
      getLocations(program);

  }, shaderType);
}

//##################################################################################################
void FullScreenShader::setFrameMatrix(const glm::mat4& frameMatrix)
{
  glUniformMatrix4fv(d->frameMatrixLocation, 1, GL_FALSE, glm::value_ptr(frameMatrix));
}

//##################################################################################################
void FullScreenShader::draw()
{
  draw(*d->object);
}

//##################################################################################################
void FullScreenShader::draw(const Object& object)
{
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  tpBindVertexArray(object.vaoID);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, object.size);
  tpBindVertexArray(0);
#else
  object.bindVBO();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, object.size);
#endif
}

//##################################################################################################
FullScreenShader::Object::Object(const Shader* shader_):
  shader(shader_)
{

}

//##################################################################################################
FullScreenShader::Object::~Object()
{
  if(!shader.shader())
    return;

  shader.shader()->map()->makeCurrent();

  glDeleteBuffers(1, &vboID);
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpDeleteVertexArrays(1, &vaoID);
#endif
}

//##################################################################################################
void FullScreenShader::Object::bindVBO()
{
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), tpVoidLiteral(0));
  glEnableVertexAttribArray(0);
}

//##################################################################################################
FullScreenShader::Object* FullScreenShader::makeObject(const std::vector<glm::vec2>& verts)
{
  FullScreenShader::Object* object = new FullScreenShader::Object(this);
  object->size = GLsizei(verts.size());

  glGenBuffers(1, &object->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, object->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(glm::vec2)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  tpGenVertexArrays(1, &object->vaoID);
  tpBindVertexArray(object->vaoID);
  object->bindVBO();
  tpBindVertexArray(0);
#endif

  return object;
}

//##################################################################################################
FullScreenShader::Object* FullScreenShader::makeRectangleObject(const glm::vec2& size)
{
  glm::vec2 z = (1.0f-size)/2.0f;
  glm::vec2 o = 1.0f - z;

  std::swap(z.y, o.y);

  std::vector<glm::vec2> verts{{z.x,z.y}, {z.x,o.y}, {o.x,z.y}, {o.x,o.y}};
  return makeObject(verts);
}

//##################################################################################################
FullScreenShader::Object* FullScreenShader::makeFrameObject(const glm::vec2& holeSize, const glm::vec2& size)
{
  glm::vec2 iz = (1.0f-holeSize)/2.0f;
  glm::vec2 io = 1.0f - iz;

  glm::vec2 oz = (1.0f-size)/2.0f;
  glm::vec2 oo = 1.0f - oz;

  std::swap(iz.y, io.y);
  std::swap(oz.y, oo.y);

  std::vector<glm::vec2> verts{{oz.x, oz.y},
                               {iz.x, iz.y},
                               {oo.x, oz.y},
                               {io.x, iz.y},
                               {oo.x, oo.y},
                               {io.x, io.y},
                               {oz.x, oo.y},
                               {iz.x, io.y},
                               {oz.x, oz.y},
                               {iz.x, iz.y}};

  return makeObject(verts);
}

}
