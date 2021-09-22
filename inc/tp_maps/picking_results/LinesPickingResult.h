#ifndef tp_maps_LinesPickingResult_h
#define tp_maps_LinesPickingResult_h

#include "tp_maps/PickingResult.h"

#include "glm/glm.hpp"

namespace tp_maps
{
class LinesLayer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT LinesPickingResult: public PickingResult
{
public:
  //################################################################################################
  LinesPickingResult(const tp_utils::StringID& pickingType_,
                      const PickingDetails& details_,
                      const RenderInfo& renderInfo_,
                      Layer* layer_,
                      size_t index_);

  size_t index;
};

}

#endif
