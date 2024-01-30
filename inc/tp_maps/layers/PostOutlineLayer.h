#ifndef tp_maps_PostOutlineLayer_h
#define tp_maps_PostOutlineLayer_h

#include "tp_maps/layers/PostSelectionLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostOutlineLayer: public PostSelectionLayer
{
public:
  //################################################################################################
  PostOutlineLayer(size_t stageMask=1, size_t stageUpdate=2);

protected:
  //################################################################################################
  PostShader* makeShader() override;
};

}

#endif
