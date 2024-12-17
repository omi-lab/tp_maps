#include "tp_maps/layers/PostSSAOLayer.h"
#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
struct PostSSAOLayer::Private
{
  Q* q;
  PostSSAOParameters parameters;

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
      q->map()->deleteShader(PostSSAOShader::name());
  }
};

//##################################################################################################
PostSSAOLayer::PostSSAOLayer():
  PostLayer({tp_maps::RenderPass::Custom, postSSAOShaderSID()}),
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

//##################################################################################################
void PostSSAOLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::SwapToFBO, postSSAOShaderSID());
  renderPasses.emplace_back(defaultRenderPass());
}

}
