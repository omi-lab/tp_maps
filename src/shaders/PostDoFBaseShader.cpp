#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace tp_maps
{
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
                                     tp_maps::OpenGLProfile openGLProfile,
                                     const PostDoFParameters& parameters):
  PostShader(map, openGLProfile),
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
