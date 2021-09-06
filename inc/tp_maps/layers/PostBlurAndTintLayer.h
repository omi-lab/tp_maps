#ifndef tp_maps_PostBlurAndTintLayer_h
#define tp_maps_PostBlurAndTintLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostBlurAndTintLayer: public PostLayer
{
public:
  //################################################################################################
  PostBlurAndTintLayer(Map* map, RenderPass customRenderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
