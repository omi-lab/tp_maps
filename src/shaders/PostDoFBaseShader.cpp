#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace tp_maps
{
//##################################################################################################
bool PostDoFParameters::fuzzyEquals(const PostDoFParameters& other) const
{
  float fuzzy = 0.001f;
  float avDepth = 0.5f*(nearPlane + farPlane);
  return other.enabled == enabled
         && std::abs(other.depthOfField-depthOfField) <= fuzzy*depthOfField
         && std::abs(other.fStop-fStop) <= fuzzy*fStop
         && std::abs(other.nearPlane-nearPlane) <= fuzzy*avDepth
         && std::abs(other.farPlane-farPlane) <= fuzzy*avDepth
         && std::abs(other.focalDistance-focalDistance) <= fuzzy*focalDistance
         && std::abs(other.blurinessCutoffConstant-blurinessCutoffConstant) <= fuzzy*blurinessCutoffConstant;
}

//##################################################################################################
struct PostDoFBaseShader::Private
{
  PostDoFParameters parameters;

  float near{  0.1f};
  float  far{100.00f};

  GLint nearLocation{0};
  GLint  farLocation{0};

  //################################################################################################
  Private(const PostDoFParameters& parameters_):
    parameters(parameters_)
  {

  }
};

//##################################################################################################
PostDoFBaseShader::PostDoFBaseShader(Map* map,
                                     tp_maps::ShaderProfile shaderProfile,
                                     const PostDoFParameters& parameters):
  PostShader(map, shaderProfile),
  d(new Private(parameters))
{
}

//##################################################################################################
PostDoFBaseShader::~PostDoFBaseShader()
{
  delete d;
}

//##################################################################################################
const PostDoFParameters& PostDoFBaseShader::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void PostDoFBaseShader::setProjectionNearAndFar(float near, float far)
{
  d->near = near;
  d->far = far;
}

//##################################################################################################
void PostDoFBaseShader::use(ShaderType shaderType)
{
  PostShader::use(shaderType);
  glUniform1f(d->nearLocation, d->near);
  glUniform1f(d-> farLocation, d-> far);
}

//##################################################################################################
void PostDoFBaseShader::getLocations(GLuint program, ShaderType shaderType)
{
  PostShader::getLocations(program, shaderType);
  d->nearLocation = glGetUniformLocation(program, "near");
  d->farLocation = glGetUniformLocation(program, "far");
}

}
