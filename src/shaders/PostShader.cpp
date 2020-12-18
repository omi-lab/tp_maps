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

  PostShader* q;

  GLuint vboID{0};

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  GLuint vaoID{0};
#endif

  GLint textureLocation {0};
  GLint depthLocation   {0};
  GLint normalsLocation {0};
  GLint specularLocation{0};

  GLint projectionMatrixLocation   {0};
  GLint invProjectionMatrixLocation{0};

  //################################################################################################
  Private(PostShader* q_, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
    q(q_)
  {
    if(!vertexShader)
      vertexShader = vertShaderStr().data(openGLProfile, ShaderType::RenderHDR);


    q->compile(vertexShader,
            fragmentShader,
            [](GLuint program)
    {
      glBindAttribLocation(program, 0, "inVertex");
    },
    [&](GLuint program)
    {
      textureLocation  = glGetUniformLocation(program, "textureSampler");
      depthLocation    = glGetUniformLocation(program, "depthSampler");
      normalsLocation  = glGetUniformLocation(program, "normalsSampler");
      specularLocation = glGetUniformLocation(program, "specularSampler");

      projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
      invProjectionMatrixLocation = glGetUniformLocation(program, "invProjectionMatrix");
    }, ShaderType::RenderHDR);

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
PostShader::PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  Shader(map, openGLProfile),
  d(new Private(this, openGLProfile, vertexShader, fragmentShader))
{

}

//##################################################################################################
PostShader::PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const std::string& fragmentShader):
  Shader(map, openGLProfile),
  d(new Private(this, openGLProfile, nullptr, fragmentShader.data()))
{

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
void PostShader::setReadFBO(const FBO& readFBO)
{
  if(d->textureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, readFBO.textureID);
    glUniform1i(d->textureLocation, 0);
  }

  if(d->depthLocation>=0)
  {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, readFBO.depthID);
    glUniform1i(d->depthLocation, 1);
  }

  if(d->normalsLocation>=0)
  {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, readFBO.normalsID);
    glUniform1i(d->normalsLocation, 2);
  }

  if(d->specularLocation>=0)
  {
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, readFBO.specularID);
    glUniform1i(d->specularLocation, 3);
  }
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
