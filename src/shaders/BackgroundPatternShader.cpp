#include "tp_maps/shaders/BackgroundPatternShader.h"
#include "tp_maps/Map.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct BackgroundPatternShader::Private
{
  GLint scaleFactorLoc{0};
};

//##################################################################################################
BackgroundPatternShader::BackgroundPatternShader(Map* map, tp_maps::ShaderProfile shaderProfile):
  FullScreenShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
BackgroundPatternShader::~BackgroundPatternShader()
{
  delete d;
}

//##################################################################################################
void BackgroundPatternShader::setScreenSizeAndGridSpacing(const glm::vec2& screenSize, float gridSpacing)
{
  glm::vec2 ss = screenSize/gridSpacing;
  glUniform2fv(d->scaleFactorLoc, 1, glm::value_ptr(ss));
}

//##################################################################################################
const char* BackgroundPatternShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/BackgroundPatternShader.frag"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
void BackgroundPatternShader::getLocations(GLuint program, ShaderType shaderType)
{
  FullScreenShader::getLocations(program, shaderType);
  d->scaleFactorLoc = glGetUniformLocation(program, "scaleFactor");
}

}
