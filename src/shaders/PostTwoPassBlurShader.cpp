#include "tp_maps/shaders/PostTwoPassBlurShader.h"

namespace tp_maps
{
//##################################################################################################
struct PostTwoPassBlurShader::Private
{
  GLint dirIndexLocation{-1};
  GLint blurRadiusLocation{-1};
  GLint blurSigmaLocation{-1};
};

//##################################################################################################
PostTwoPassBlurShader::PostTwoPassBlurShader(tp_maps::Map* map, tp_maps::ShaderProfile shaderProfile):
  PostShader(map, shaderProfile),
  d(new Private())
{
}

//##################################################################################################
PostTwoPassBlurShader::~PostTwoPassBlurShader()
{
  delete d;
}
//##################################################################################################
void PostTwoPassBlurShader::use(tp_maps::ShaderType shaderType)
{
  tp_maps::PostShader::use(shaderType);

  assert(dirIndex == 0 || dirIndex == 1);
  glUniform1i(d->dirIndexLocation  , std::clamp(dirIndex, 0, 1));
  glUniform1f(d->blurRadiusLocation, blurRadius);
  glUniform1f(d->blurSigmaLocation , 2.0f * blurSigma * blurSigma);
}

//##################################################################################################
const std::string& PostTwoPassBlurShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/PostTwoPassBlurShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

//##################################################################################################
void PostTwoPassBlurShader::getLocations(GLuint program, tp_maps::ShaderType shaderType)
{
  tp_maps::PostShader::getLocations(program, shaderType);
  d->dirIndexLocation   = glGetUniformLocation(program, "dirIndex"  );
  d->blurRadiusLocation = glGetUniformLocation(program, "blurRadius");
  d->blurSigmaLocation  = glGetUniformLocation(program, "blurSigma" );
}
}
