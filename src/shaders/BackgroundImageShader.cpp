#include "tp_maps/shaders/BackgroundImageShader.h"
#include "tp_maps/Map.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct BackgroundImageShader::Private
{
  GLint textureLocation{0};
  GLint matrixLocation{0};
};

//##################################################################################################
BackgroundImageShader::BackgroundImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  FullScreenShader(map, openGLProfile),
  d(new Private())
{

}

//##################################################################################################
BackgroundImageShader::~BackgroundImageShader()
{
  delete d;
}

//##################################################################################################
void BackgroundImageShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(d->textureLocation, 0);
}

//##################################################################################################
void BackgroundImageShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
const char* BackgroundImageShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/BackgroundImageShader.vert"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
const char* BackgroundImageShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/BackgroundImageShader.frag"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
void BackgroundImageShader::getLocations(GLuint program, ShaderType shaderType)
{
  FullScreenShader::getLocations(program, shaderType);
  d->textureLocation = glGetUniformLocation(program, "textureSampler");
  d->matrixLocation  = glGetUniformLocation(program, "matrix");
}

}
