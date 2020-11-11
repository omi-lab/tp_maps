#include "tp_maps/shaders/PostShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/PostShader.vert"}; return s;}
}

//##################################################################################################
struct PostShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::PostShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLuint vboID{0};

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  GLuint vaoID{0};
#endif

  GLint textureLocation{0};
  GLint depthLocation  {0};

  GLint projectionMatrixLocation   {0};
  GLint invProjectionMatrixLocation{0};

  //################################################################################################
  void bindVBO()
  {
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), tpVoidLiteral(0));
    glEnableVertexAttribArray(0);
  }
};

//##################################################################################################
PostShader::PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  Shader(map, openGLProfile),
  d(new Private())
{
  if(!vertexShader)
    vertexShader = vertShaderStr().data(openGLProfile);


  compile(vertexShader,
          fragmentShader,
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
  },
  [&](GLuint program)
  {
    d->textureLocation = glGetUniformLocation(program, "textureSampler");
    d->depthLocation   = glGetUniformLocation(program, "depthSampler");

    d->projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
    d->invProjectionMatrixLocation = glGetUniformLocation(program, "invProjectionMatrix");
  });

  std::vector<glm::vec2> verts{{0,0}, {1,0}, {1,1}, {0,1}};

  glGenBuffers(1, &d->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, d->vboID);
  glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(glm::vec2)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  tpGenVertexArrays(1, &d->vaoID);
  tpBindVertexArray(d->vaoID);
  d->bindVBO();
  tpBindVertexArray(0);
#endif
}

//##################################################################################################
PostShader::~PostShader()
{
  delete d;
}

//##################################################################################################
void PostShader::use(ShaderType shaderType)
{
  Shader::use(shaderType);
}

//##################################################################################################
void PostShader::setReflectionTextures(GLuint colorID, GLuint depthID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, colorID);
  glUniform1i(d->textureLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, depthID);
  glUniform1i(d->depthLocation, 1);
}

//##################################################################################################
void PostShader::setProjectionMatrix(const glm::mat4& projectionMatrix)
{
  glm::mat4 invProjectionMatrix = glm::inverse(projectionMatrix);
  glUniformMatrix4fv(d->projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
  glUniformMatrix4fv(d->invProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(invProjectionMatrix));
}

//##################################################################################################
void PostShader::draw()
{
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(d->vaoID);
    //tpDrawElements(GL_TRIANGLE_FAN, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif
}


}
