#ifndef tp_maps_PostViewTransformLayer_h
#define tp_maps_PostViewTransformLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostViewTransformLayer: public PostLayer
{
public:
  //################################################################################################
  PostViewTransformLayer();

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
