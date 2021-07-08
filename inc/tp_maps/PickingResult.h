#ifndef tp_maps_PickingResult_h
#define tp_maps_PickingResult_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
struct PickingDetails;
class RenderInfo;
class Layer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PickingResult
{
  TP_NONCOPYABLE(PickingResult);
public:
  //################################################################################################
  PickingResult(const tp_utils::StringID& pickingType_,
                const PickingDetails& details_,
                const RenderInfo& renderInfo_,
                Layer* layer_);

  //################################################################################################
  virtual ~PickingResult()=default;

  const tp_utils::StringID& pickingType;
  const PickingDetails& details;
  const RenderInfo& renderInfo;  
  Layer* layer;
};

}

#endif
