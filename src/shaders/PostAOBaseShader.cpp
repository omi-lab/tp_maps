#include "tp_maps/shaders/PostAOBaseShader.h"

namespace tp_maps
{

//##################################################################################################
struct PostAOBaseShader::Private
{
  PostAOParameters parameters;

  //################################################################################################
  Private(const PostAOParameters& parameters_):
    parameters(parameters_)
  {

  }
};

//##################################################################################################
PostAOBaseShader::PostAOBaseShader(Map* map,
                                   tp_maps::ShaderProfile shaderProfile,
                                   const PostAOParameters& parameters):
  PostShader(map, shaderProfile),
  d(new Private(parameters))
{

}

//##################################################################################################
PostAOBaseShader::~PostAOBaseShader()
{
  delete d;
}

//##################################################################################################
const PostAOParameters& PostAOBaseShader::parameters() const
{
  return d->parameters;
}

}
