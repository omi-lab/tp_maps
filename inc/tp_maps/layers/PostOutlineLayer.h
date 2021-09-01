#ifndef tp_maps_PostOutlineLayer_h
#define tp_maps_PostOutlineLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostOutlineLayer: public PostLayer
{
public:
  //################################################################################################
  PostOutlineLayer(Map* map, RenderPass customRenderPass1, RenderPass customRenderPass2);

  //################################################################################################
  ~PostOutlineLayer();

  //################################################################################################
  //! The pass that the mask is drawn in.
  /*!
  The mask is used to calculate the outline.

  The depth buffer is used as the mask and the outline is drawn using the following rule:
   - If a pixel is not set and is less than n away from a set pixel it will be drawn as the outline
     color else it is discarded.

  Other layers should also draw in this pass as well as their default pass if they wish to be
  outlined.
  */
  RenderPass customRenderPass1() const;

  //################################################################################################
  //! The pass that the outline is drawn in.
  RenderPass customRenderPass2() const;


protected:
  //################################################################################################
  PostShader* makeShader() override;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
