#ifndef tp_maps_ImagePickingResult_h
#define tp_maps_ImagePickingResult_h

#include "tp_maps/PickingResult.h"

namespace tp_maps
{
struct PickingDetails;
class RenderInfo;
class ImageLayer;

//##################################################################################################
class TP_MAPS_EXPORT ImagePickingResult: public PickingResult
{
public:
  //################################################################################################
  ImagePickingResult(const tp_utils::StringID& pickingType_,
                     const PickingDetails& details_,
                     const RenderInfo& renderInfo_,
                     ImageLayer* imageLayer_,
                     int imageX_,
                     int imageY_);

  const int imageX;
  const int imageY;
};

}

#endif
