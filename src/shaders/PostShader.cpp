#include "tp_maps/shaders/PostShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct PostShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::detail::PostShaderPrivate::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint textureLocation {0};
  GLint depthLocation   {0};
  GLint normalsLocation {0};
  GLint specularLocation{0};

  GLint projectionMatrixLocation   {0};
  GLint invProjectionMatrixLocation{0};

  //################################################################################################
  std::function<void(GLuint)> bindLocations()
  {
    return std::function<void(GLuint)>();
  }

  //################################################################################################
  std::function<void(GLuint)> getLocations()
  {
    return [&](GLuint program)
    {
      textureLocation  = glGetUniformLocation(program, "textureSampler");
      depthLocation    = glGetUniformLocation(program, "depthSampler");
      normalsLocation  = glGetUniformLocation(program, "normalsSampler");
      specularLocation = glGetUniformLocation(program, "specularSampler");

      projectionMatrixLocation = glGetUniformLocation(program, "projectionMatrix");
      invProjectionMatrixLocation = glGetUniformLocation(program, "invProjectionMatrix");
    };
  }
};

//##################################################################################################
PostShader::PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  FullScreenShader(map, openGLProfile),
  d(new Private())
{
  compile(vertexShader, fragmentShader, d->bindLocations(), d->getLocations(), ShaderType::RenderExtendedFBO);
}

//##################################################################################################
PostShader::~PostShader()
{
  delete d;
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

}
