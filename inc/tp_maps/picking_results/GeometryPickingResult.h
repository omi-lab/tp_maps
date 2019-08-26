#ifndef tp_maps_GeometryPickingResult_h
#define tp_maps_GeometryPickingResult_h

#include "tp_maps/PickingResult.h"

namespace tp_maps
{
struct PickingDetails;
class RenderInfo;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GeometryPickingResult: public PickingResult
{
public:
  //################################################################################################
  GeometryPickingResult(const tp_utils::StringID& pickingType_,
                        const PickingDetails& details_,
                        const RenderInfo& renderInfo_);

  //################################################################################################
  ~GeometryPickingResult()override;

  const int geometryIndex;
};

}

#endif
