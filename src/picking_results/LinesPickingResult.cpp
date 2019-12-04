#include "tp_maps/picking_results/LinesPickingResult.h"

namespace tp_maps
{

//##################################################################################################
LinesPickingResult::LinesPickingResult(const tp_utils::StringID& pickingType_,
                                         const PickingDetails& details_,
                                         const RenderInfo& renderInfo_,
                                         LinesLayer* linesLayer_,
                                         size_t index_):
  PickingResult(pickingType_, details_, renderInfo_),
  linesLayer(linesLayer_),
  index(index_)
{

}

}
