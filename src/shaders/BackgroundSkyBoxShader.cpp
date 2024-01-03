#include "tp_maps/shaders/BackgroundSkyBoxShader.h"
#include "tp_maps/Map.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct BackgroundSkyBoxShader::Private
{
  GLint textureLocation{0};
  GLint vpInvMatrixLocation{0};
  GLint rotationFactorLocation{0};
};

//##################################################################################################
BackgroundSkyBoxShader::BackgroundSkyBoxShader(Map* map, tp_maps::ShaderProfile shaderProfile):
  FullScreenShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
BackgroundSkyBoxShader::~BackgroundSkyBoxShader()
{
  delete d;
}

//##################################################################################################
void BackgroundSkyBoxShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(d->textureLocation, 0);
}

//##################################################################################################
void BackgroundSkyBoxShader::setMatrix(const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 vp = p*v;
  glm::mat4 vpInv = glm::inverse(vp);
  glUniformMatrix4fv(d->vpInvMatrixLocation, 1, GL_FALSE, glm::value_ptr(vpInv));
}

//##################################################################################################
void BackgroundSkyBoxShader::setRotationFactor(float rotationFactor)
{
  glUniform1f(d->rotationFactorLocation, rotationFactor);
}

//##################################################################################################
const char* BackgroundSkyBoxShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/BackgroundSkyBoxShader.frag"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
void BackgroundSkyBoxShader::getLocations(GLuint program, ShaderType shaderType)
{
  FullScreenShader::getLocations(program, shaderType);
  d->textureLocation        = glGetUniformLocation(program, "textureSampler");
  d->vpInvMatrixLocation    = glGetUniformLocation(program, "vpInv");
  d->rotationFactorLocation = glGetUniformLocation(program, "rotationFactor");
}

}
