#include "tp_maps/shaders/BackgroundShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/BackgroundShader.frag"}; return s;}

//##################################################################################################
struct UniformLocations_lt
{
  GLint textureLocation{0};
  GLint vpInvMatrixLocation{0};
  GLint rotationFactorLocation{0};
};
}

//##################################################################################################
struct BackgroundShader::Private
{
  std::unordered_map<ShaderType, UniformLocations_lt> locations;

  //################################################################################################
  std::function<void(GLuint)> getLocations(ShaderType shaderType)
  {
    return [=](GLuint program)
    {
      auto& l = locations[shaderType];
      l.textureLocation     = glGetUniformLocation(program, "textureSampler");
      l.vpInvMatrixLocation = glGetUniformLocation(program, "vpInv");
      l.rotationFactorLocation = glGetUniformLocation(program, "rotationFactor");
    };
  }
};

//##################################################################################################
BackgroundShader::BackgroundShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  FullScreenShader(map, openGLProfile),
  d(new Private())
{
  auto comp = [&](ShaderType shaderType)
  {
    compile(nullptr,
            fragShaderStr().data(openGLProfile, shaderType),
            std::function<void(GLuint)>(),
            d->getLocations(shaderType),
            shaderType);
  };

  comp(ShaderType::Render);
  comp(ShaderType::RenderExtendedFBO);
}

//##################################################################################################
BackgroundShader::~BackgroundShader()
{
  delete d;
}

//##################################################################################################
void BackgroundShader::setTexture(GLuint textureID)
{
  auto& l = d->locations[currentShaderType()];

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(l.textureLocation, 0);
}

//##################################################################################################
void BackgroundShader::setMatrix(const glm::mat4& v, const glm::mat4& p)
{
  auto& l = d->locations[currentShaderType()];

  glm::mat4 vp = p*v;
  glm::mat4 vpInv = glm::inverse(vp);
  glUniformMatrix4fv(l.vpInvMatrixLocation, 1, GL_FALSE, glm::value_ptr(vpInv));
}

//##################################################################################################
void BackgroundShader::setRotationFactor(float rotationFactor)
{
  auto& l = d->locations[currentShaderType()];
  glUniform1f(l.rotationFactorLocation, rotationFactor);
}

}
