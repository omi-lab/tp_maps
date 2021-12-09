#include "tp_maps/shaders/PatternShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/PatternShader.frag"}; return s;}

//##################################################################################################
struct UniformLocations_lt
{
  GLint scaleFactorLoc{0};
};
}

//##################################################################################################
struct PatternShader::Private
{
  std::unordered_map<ShaderType, UniformLocations_lt> locations;

  //################################################################################################
  std::function<void(GLuint)> getLocations(ShaderType shaderType)
  {
    return [=](GLuint program)
    {
      auto& l = locations[shaderType];
      l.scaleFactorLoc = glGetUniformLocation(program, "scaleFactor");
    };
  }
};

//##################################################################################################
PatternShader::PatternShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
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
PatternShader::~PatternShader()
{
  delete d;
}

//##################################################################################################
void PatternShader::setScreenSizeAndGridSpacing(const glm::vec2& screenSize, float gridSpacing)
{
  glm::vec2 ss = screenSize/gridSpacing;
  glUniform2fv(d->locations[currentShaderType()].scaleFactorLoc, 1, glm::value_ptr(ss));
}

}
