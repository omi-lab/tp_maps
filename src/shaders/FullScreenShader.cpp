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

  GLuint vboID{0};

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  GLuint vaoID{0};
#endif

  //################################################################################################
  Private()
  {
    std::vector<glm::vec2> verts{{0,0}, {1,0}, {1,1}, {0,1}};

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(glm::vec2)), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpGenVertexArrays(1, &vaoID);
    tpBindVertexArray(vaoID);
    bindVBO();
    tpBindVertexArray(0);
#endif
  }

  //################################################################################################
  void bindVBO()
  {
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), tpVoidLiteral(0));
    glEnableVertexAttribArray(0);
  }
};

//##################################################################################################
FullScreenShader::FullScreenShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Shader(map, openGLProfile),
  d(new Private())
{

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
  }, getLocations, shaderType);
}

//##################################################################################################
void FullScreenShader::draw()
{
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  tpBindVertexArray(d->vaoID);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  tpBindVertexArray(0);
#else
  vertexBuffer->bindVBO();
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
}


}
