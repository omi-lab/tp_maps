#include "tp_maps/PickingResult.h"

namespace tp_maps
{

//##################################################################################################
PickingResult::PickingResult(const tp_utils::StringID& pickingType_,
                const PickingDetails& details_,
                const RenderInfo& renderInfo_,
                             Layer* layer_):
  pickingType(pickingType_),
  details(details_),
  renderInfo(renderInfo_),
  layer(layer_)
{

}

}
