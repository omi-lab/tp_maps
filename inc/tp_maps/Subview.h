#ifndef tp_maps_Subview_h
#define tp_maps_Subview_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;

//##################################################################################################
//! A sub view within the Map.
class TP_MAPS_EXPORT Subview
{
  friend class Map;
  TP_NONCOPYABLE(Subview);
public:
  //################################################################################################
  Subview(Map* map, const tp_utils::StringID& name);

  //################################################################################################
  ~Subview();

  //################################################################################################
  const tp_utils::StringID& name() const;

private:  
  //################################################################################################
  void setRenderPassesInternal(const std::vector<RenderPass>& renderPasses);

  //################################################################################################
  bool hasRenderPass(RenderPass::RenderPassType renderPassType);

  //################################################################################################
  void deletePostLayers();

  Map* m_map;
  const tp_utils::StringID m_name;

  std::vector<RenderPass> m_renderPasses;
  std::vector<RenderPass> m_computedRenderPasses;  

  size_t m_width{1};
  size_t m_height{1};

  RenderFromStage m_renderFromStage{RenderFromStage::Full};
};

}

#endif
