#ifndef tp_maps_DefaultSpritesTexture_h
#define tp_maps_DefaultSpritesTexture_h

#include "tp_maps/textures/BasicTexture.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT DefaultSpritesTexture : public BasicTexture
{
  TP_REF_COUNT_OBJECTS("DefaultSpritesTexture");
public:
  //################################################################################################
  DefaultSpritesTexture(Map* map);
};

}

#endif
