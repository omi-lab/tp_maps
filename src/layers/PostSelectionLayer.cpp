#include "tp_maps/layers/PostSelectionLayer.h"

namespace tp_maps
{

//##################################################################################################
struct PostSelectionLayer::Private
{
  tp_utils::StringID selectionOutput{"Selection output"};
  tp_maps::RenderPass selectionRenderPass{PostSelectionLayer::selectionRenderPass()};
  tp_maps::RenderFromStage renderFromStageMask, renderFromStageUpdate;

  //################################################################################################
  Private(size_t stageMask, size_t stageUpdate):
    renderFromStageMask(tp_maps::RenderFromStage::Stage, stageMask),
    renderFromStageUpdate(tp_maps::RenderFromStage::Stage, stageUpdate)
  {

  }
};

//##################################################################################################
PostSelectionLayer::PostSelectionLayer(const RenderPass& customRenderPass, size_t stageMask, size_t stageUpdate):
  PostLayer(customRenderPass),
  d(new Private(stageMask, stageUpdate))
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
tp_maps::RenderFromStage PostSelectionLayer::renderFromStageMask() const
{
  return d->renderFromStageMask;
}

//##################################################################################################
tp_maps::RenderFromStage PostSelectionLayer::renderFromStageUpdate() const
{
  return d->renderFromStageUpdate;
}

//##################################################################################################
void PostSelectionLayer::addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses)
{
  if(bypass())
    return;

  if(!containsPass(renderPasses, d->selectionRenderPass))
  {
    auto inputFBO = findInputFBO(renderPasses);
    renderPasses.emplace_back(d->renderFromStageMask);
    renderPasses.emplace_back(tp_maps::RenderPass::SwapToFBO, tp_maps::selectionPassSID());
    renderPasses.emplace_back(d->selectionRenderPass);
    renderPasses.emplace_back(d->renderFromStageUpdate);
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
