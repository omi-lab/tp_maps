#ifndef tp_maps_HandlePickingResult_h
#define tp_maps_HandlePickingResult_h

#include "tp_maps/PickingResult.h"

namespace tp_maps
{
class HandleDetails;

//##################################################################################################
class HandlePickingResult: public PickingResult
{
public:
  //################################################################################################
  HandlePickingResult(const tp_utils::StringID& pickingType_,
                     const PickingDetails& details_,
                     const RenderInfo& renderInfo_,
                     HandleDetails* handle_);

  const HandleDetails* handle;
};

}

#endif
