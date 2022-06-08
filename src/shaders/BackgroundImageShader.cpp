#include "tp_maps/shaders/BackgroundImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/BackgroundImageShader.frag"}; return s;}
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/BackgroundImageShader.vert"}; return s;}

//##################################################################################################
struct UniformLocations_lt
{
  GLint textureLocation{0};
  GLint matrixLocation{0};
};
}

//##################################################################################################
struct BackgroundImageShader::Private
{
  std::unordered_map<ShaderType, UniformLocations_lt> locations;

  //################################################################################################
  std::function<void(GLuint)> getLocations(ShaderType shaderType)
  {
    return [=](GLuint program)
    {
      auto& l = locations[shaderType];
      l.textureLocation     = glGetUniformLocation(program, "textureSampler");
      l.matrixLocation = glGetUniformLocation(program, "matrix");
    };
  }
};

//##################################################################################################
BackgroundImageShader::BackgroundImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  FullScreenShader(map, openGLProfile),
  d(new Private())
{
  auto comp = [&](ShaderType shaderType)
  {
    compile(vertShaderStr().data(openGLProfile, shaderType),
            fragShaderStr().data(openGLProfile, shaderType),
            std::function<void(GLuint)>(),
            d->getLocations(shaderType),
            shaderType);
  };

  comp(ShaderType::Render);
  comp(ShaderType::RenderExtendedFBO);
}

//##################################################################################################
BackgroundImageShader::~BackgroundImageShader()
{
  delete d;
}

//##################################################################################################
void BackgroundImageShader::setTexture(GLuint textureID)
{
  auto& l = d->locations[currentShaderType()];

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glUniform1i(l.textureLocation, 0);
}

//##################################################################################################
void BackgroundImageShader::setMatrix(const glm::mat4& matrix)
{
  auto& l = d->locations[currentShaderType()];
  glUniformMatrix4fv(l.matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

}
