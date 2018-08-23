#include "tp_maps/picking_results/PointsPickingResult.h"

namespace tp_maps
{

//##################################################################################################
PointsPickingResult::PointsPickingResult(const tp_utils::StringID& pickingType_,
                                         const PickingDetails& details_,
                                         const RenderInfo& renderInfo_,
                                         PointsLayer* pointsLayer_,
                                         size_t index_,
                                         const PointSpriteShader::PointSprite& pointSprite_):
  PickingResult(pickingType_, details_, renderInfo_),
  pointsLayer(pointsLayer_),
  index(index_),
  pointSprite(pointSprite_)
{

}

}
