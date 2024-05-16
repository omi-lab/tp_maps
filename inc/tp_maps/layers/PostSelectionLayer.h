#ifndef tp_maps_PostSelectionLayer_h
#define tp_maps_PostSelectionLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
//! Base class for layers that draw the current selection
class TP_MAPS_EXPORT PostSelectionLayer: public PostLayer
{
  TP_DQ;
public:
  //################################################################################################
  PostSelectionLayer(const RenderPass& customRenderPass, size_t stageMask=1, size_t stageUpdate=2);

  //################################################################################################
  ~PostSelectionLayer();

  //################################################################################################
  static tp_maps::RenderPass selectionRenderPass();

  //################################################################################################
  //! Render from stage to render mask
  tp_maps::RenderFromStage renderFromStageMask() const;

  //################################################################################################
  //! Render from stage to update state given mask already rendered
  tp_maps::RenderFromStage renderFromStageUpdate() const;

protected:
  //################################################################################################
  void addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses) override;

  //################################################################################################
  void prepareForRenderPass(const tp_maps::RenderPass& renderPass) override;
};

}

#endif
