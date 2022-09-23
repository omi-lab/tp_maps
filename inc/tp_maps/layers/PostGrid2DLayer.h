#ifndef tp_maps_PostGrid2DLayer_h
#define tp_maps_PostGrid2DLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostGrid2DLayer: public PostLayer
{
public:
  //################################################################################################
  PostGrid2DLayer(RenderPass::RenderPassType renderPass);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
