#include "tp_maps/layers/PostSSAOLayer.h"
#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
struct PostSSAOLayer::Private
{
  PostSSAOLayer* q;
  PostSSAOParameters parameters;

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
      q->map()->deleteShader(PostSSAOShader::name());
  }

  //################################################################################################
  Private(PostSSAOLayer* q_):
    q(q_)
  {

  }
};

//##################################################################################################
PostSSAOLayer::PostSSAOLayer(RenderPass::RenderPassType customRenderPass):
  PostLayer(customRenderPass),
  d(new Private(this))
{

}

//##################################################################################################
PostSSAOLayer::~PostSSAOLayer()
{
  delete d;
}

//##################################################################################################
const PostSSAOParameters& PostSSAOLayer::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void PostSSAOLayer::setParameters(const PostSSAOParameters& parameters)
{
  d->parameters = parameters;
  d->recompileShaders();
}

//##################################################################################################
PostShader* PostSSAOLayer::makeShader()
{
  return map()->getShader<PostSSAOShader>(d->parameters);
}

}
