#ifndef tp_maps_PostGammaLayer_h
#define tp_maps_PostGammaLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostGammaLayer: public PostLayer
{
public:
  //################################################################################################
  PostGammaLayer();

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
