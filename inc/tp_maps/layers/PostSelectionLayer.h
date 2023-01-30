#ifndef tp_maps_PostSelectionLayer_h
#define tp_maps_PostSelectionLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
//! Base class for layers that draw the current selection
class TP_MAPS_EXPORT PostSelectionLayer: public PostLayer
{
public:
  //################################################################################################
  PostSelectionLayer(const RenderPass& customRenderPass, size_t stage=1);

  //################################################################################################
  ~PostSelectionLayer();

  //################################################################################################
  static tp_maps::RenderPass selectionRenderPass();

protected:
  //################################################################################################
  void addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses) override;

  //################################################################################################
  void prepareForRenderPass(const tp_maps::RenderPass& renderPass) override;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
