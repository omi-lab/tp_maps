#include "tp_maps/picking_results/LinesPickingResult.h"
#include "tp_maps/layers/LinesLayer.h"

namespace tp_maps
{

//##################################################################################################
LinesPickingResult::LinesPickingResult(const tp_utils::StringID& pickingType_,
                                         const PickingDetails& details_,
                                         const RenderInfo& renderInfo_,
                                         Layer* layer_,
                                         size_t index_):
  PickingResult(pickingType_, details_, renderInfo_, layer_),
  index(index_)
{

}

}
