#ifndef tp_maps_PostOutlineLayer_h
#define tp_maps_PostOutlineLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostOutlineLayer: public PostLayer
{
public:
  //################################################################################################
  PostOutlineLayer();

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
