#include "tp_maps/layers/PostSelectionLayer.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
struct PostSelectionLayer::Private
{
  size_t selectedCount{0};
};


//##################################################################################################
PostSelectionLayer::PostSelectionLayer():
  PostLayer(RenderPass(tp_maps::RenderPass::Custom, tp_maps::selectionPassSID())),
  d(new Private())
{

}

//##################################################################################################
PostSelectionLayer::~PostSelectionLayer()
{
  delete d;
}

//##################################################################################################
const tp_utils::StringID& PostSelectionLayer::selectionMaskFBO() const
{
  return tp_maps::selectionPassSID();
}

//##################################################################################################
void PostSelectionLayer::incrementSelectedCount()
{
  d->selectedCount++;

  if(d->selectedCount == 1)
    setBypass(false);
  else
    update(stage());

  selectedCountChanged();
}

//##################################################################################################
void PostSelectionLayer::decrementSelectedCount()
{
  d->selectedCount--;

  if(d->selectedCount == 0)
    setBypass(false);
  else
    update(stage());

  selectedCountChanged();
}

//##################################################################################################
size_t PostSelectionLayer::selectedCount() const
{
  return d->selectedCount;
}

//##################################################################################################
void PostSelectionLayer::addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(tp_maps::RenderPass::PushFBOs);
  renderPasses.emplace_back(tp_maps::RenderPass::SwapToFBO, selectionMaskFBO());
  renderPasses.emplace_back(defaultRenderPass());  
  renderPasses.emplace_back(tp_maps::RenderPass::PopFBOs);
}

//##################################################################################################
void PostSelectionLayer::prepareForRenderPass(const tp_maps::RenderPass& renderPass)
{
  TP_UNUSED(renderPass);
  glDepthMask(true);
  glEnable(GL_DEPTH_TEST);
}

//##################################################################################################
void PostSelectionLayer::render(RenderInfo& renderInfo)
{
  TP_UNUSED(renderInfo);
}

}
