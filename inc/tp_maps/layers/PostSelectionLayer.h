#ifndef tp_maps_PostSelectionLayer_h
#define tp_maps_PostSelectionLayer_h

#include "tp_maps/layers/PostLayer.h"
#include "tp_maps/LayerPointer.h"

namespace tp_maps
{

//##################################################################################################
//! Base class for layers that draw the current selection
class TP_MAPS_EXPORT PostSelectionLayer: public PostLayer
{
  TP_DQ;
public:
  //################################################################################################
  PostSelectionLayer();

  //################################################################################################
  ~PostSelectionLayer();

  //################################################################################################
  //! The name of the FBO containing the selected objects after they have been rendered.
  const tp_utils::StringID& selectionMaskFBO() const;

  //################################################################################################
  void incrementSelectedCount();

  //################################################################################################
  void decrementSelectedCount();

  //################################################################################################
  size_t selectedCount() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> selectedCountChanged;

protected:
  //################################################################################################
  void addRenderPasses(std::vector<tp_maps::RenderPass>& renderPasses) override;

  //################################################################################################
  void prepareForRenderPass(const tp_maps::RenderPass& renderPass) override;

  //################################################################################################
  void render(RenderInfo& renderInfo) override;
};

//##################################################################################################
struct SelectionHelper
{
  TP_NONCOPYABLE(SelectionHelper);

  //################################################################################################
  SelectionHelper(PostSelectionLayer* layer):
    m_layer(layer)
  {
    if(m_layer.layer())
      static_cast<PostSelectionLayer*>(m_layer.layer())->incrementSelectedCount();
  }

  //################################################################################################
  ~SelectionHelper()
  {
    if(m_layer.layer())
      static_cast<PostSelectionLayer*>(m_layer.layer())->decrementSelectedCount();
  }

private:
  LayerPointer m_layer;
};

}

#endif
