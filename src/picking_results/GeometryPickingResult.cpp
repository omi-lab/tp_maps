#include "tp_maps/picking_results/GeometryPickingResult.h"
#include "tp_maps/RenderInfo.h"

namespace tp_maps
{

//##################################################################################################
GeometryPickingResult::GeometryPickingResult(const tp_utils::StringID& pickingType_,
                                             const PickingDetails& details_,
                                             const RenderInfo& renderInfo_):
  PickingResult(pickingType_, details_, renderInfo_),
  geometryIndex(details_.index)
{

}

//##################################################################################################
GeometryPickingResult::~GeometryPickingResult() = default;

}
