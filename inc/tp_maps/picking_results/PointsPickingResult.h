#ifndef tp_maps_PointsPickingResult_h
#define tp_maps_PointsPickingResult_h

#include "tp_maps/PickingResult.h"
#include "tp_maps/shaders/PointSpriteShader.h"

#include <glm/glm.hpp>

namespace tp_maps
{
class PointsLayer;

//##################################################################################################
class PointsPickingResult: public PickingResult
{
public:
  //################################################################################################
  PointsPickingResult(const tp_utils::StringID& pickingType_,
                      const PickingDetails& details_,
                      const RenderInfo& renderInfo_,
                      PointsLayer* pointsLayer_,
                      size_t index_,
                      const PointSpriteShader::PointSprite& pointSprite_);

  PointsLayer* pointsLayer;
  size_t index;
  PointSpriteShader::PointSprite pointSprite;
};

}

#endif
