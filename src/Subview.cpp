#include "tp_maps/Subview.h"
#include "tp_maps/Map.h"
#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{
class Map;

//##################################################################################################
Subview::Subview(Map* map, const tp_utils::StringID& name):
  m_map(map),
  m_name(name)
{

}

//##################################################################################################
Subview::~Subview()
{
  deletePostLayers();
}

//##################################################################################################
const tp_utils::StringID& Subview::name() const
{
  return m_name;
}

//##################################################################################################
Controller* Subview::controller() const
{
  return m_controller;
}

//##################################################################################################
void Subview::setRenderPassesInternal(const std::vector<RenderPass>& renderPasses)
{
  deletePostLayers();

  m_renderPasses = renderPasses;
  for(const auto& renderPass : m_renderPasses)
  {
    if(renderPass.postLayer)
    {
      renderPass.postLayer->setOnlyInSubviews({m_name});
      m_map->insertLayer(0, renderPass.postLayer);
    }
  }
}

//##################################################################################################
bool Subview::hasRenderPass(RenderPass::RenderPassType renderPassType)
{
  for(const auto& renderPass : m_renderPasses)
    if(renderPass.type == renderPassType)
      return true;
  return false;
}

//##################################################################################################
void Subview::deletePostLayers()
{
  for(const auto& renderPass : m_renderPasses)
    delete renderPass.postLayer;
  m_renderPasses.clear();
}

}
