#ifndef tp_maps_PostGammaLayer_h
#define tp_maps_PostGammaLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostGammaLayer: public PostLayer
{
public:
  //################################################################################################
  PostGammaLayer(Map* map, RenderPass customRenderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
