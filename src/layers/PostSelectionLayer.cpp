#include "tp_maps/layers/PostSelectionLayer.h"

namespace tp_maps
{

//##################################################################################################
struct PostSelectionLayer::Private
{
  tp_utils::StringID selectionOutput{"Selection output"};
  tp_maps::RenderPass selectionRenderPass{PostSelectionLayer::selectionRenderPass()};
  tp_maps::RenderFromStage renderFromStage;

  //################################################################################################
  Private(size_t stage):
    renderFromStage(tp_maps::RenderFromStage::Stage, stage)
  {

  }
};

//##################################################################################################
PostSelectionLayer::PostSelectionLayer(const RenderPass& customRenderPass, size_t stage):
  PostLayer(customRenderPass),
  d(new Private(stage))
{
  d->selectionRenderPass.postLayer = this;
}

//##################################################################################################
PostSelectionLayer::~PostSelectionLayer()
{
  delete d;
}

//##################################################################################################
tp_maps::RenderPass PostSelectionLayer::selectionRenderPass()
{
  return {tp_maps::RenderPass::Custom, tp_maps::selectionPassSID()};
}

//##################################################################################################
void PostSelectionLayer::addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses)
{
  if(!containsPass(renderPasses, d->selectionRenderPass))
  {
    auto inputFBO = findInputFBO(renderPasses);
    renderPasses.emplace_back(tp_maps::RenderPass::SwapToFBO, tp_maps::selectionPassSID());
    renderPasses.emplace_back(d->selectionRenderPass);
    renderPasses.emplace_back(d->renderFromStage);
    renderPasses.emplace_back(tp_maps::RenderPass::SwapToFBO, d->selectionOutput);
    renderPasses.emplace_back(tp_maps::RenderPass::BlitFromFBO, inputFBO);
  }

  renderPasses.emplace_back(defaultRenderPass());
}

//##################################################################################################
void PostSelectionLayer::prepareForRenderPass(const tp_maps::RenderPass& renderPass)
{
  if(d->selectionRenderPass == renderPass)
  {
    glDepthMask(true);
    glEnable(GL_DEPTH_TEST);
  }
  else
    tp_maps::PostLayer::prepareForRenderPass(renderPass);
}

}
