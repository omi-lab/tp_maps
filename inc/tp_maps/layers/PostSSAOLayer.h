#ifndef tp_maps_PostSSAOLayer_h
#define tp_maps_PostSSAOLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostSSAOLayer: public PostLayer
{
public:
  //################################################################################################
  PostSSAOLayer(Map* map, RenderPass customRenderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
