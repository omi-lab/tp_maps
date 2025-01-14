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
size_t Subview::width() const
{
  return m_width;
}

//##################################################################################################
size_t Subview::height() const
{
  return m_height;
}

//##################################################################################################
size_t Subview::getStageIndex(const tp_utils::StringID& stageName) const
{
  auto i = m_stageNameToIndex.find(stageName);
  return i!=m_stageNameToIndex.end()?i->second:size_t(0);
}

//##################################################################################################
void Subview::setRenderPassesInternal(const std::vector<RenderPass>& renderPasses)
{
  deletePostLayers();

  m_stageNameToIndex.clear();

  m_renderPasses = renderPasses;
  for(size_t i=0; i<m_renderPasses.size(); i++)
  {
    auto& renderPass = m_renderPasses.at(i);

    if(renderPass.type == RenderPass::Stage)
    {
      renderPass.index = i;
      m_stageNameToIndex[renderPass.name] = i;
    }

    if(renderPass.postLayer)
    {
      renderPass.postLayer->setStageIndex(i);
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
