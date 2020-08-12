#include "tp_maps/picking_results/ImagePickingResult.h"
#include "tp_maps/layers/ImageLayer.h"

namespace tp_maps
{

//##################################################################################################
ImagePickingResult::ImagePickingResult(const tp_utils::StringID& pickingType_,
                                       const PickingDetails& details_,
                                       const RenderInfo& renderInfo_,
                                       ImageLayer* imageLayer_,
                                       int imageX_,
                                       int imageY_):
  PickingResult(pickingType_, details_, renderInfo_, imageLayer_),
  imageX(imageX_),
  imageY(imageY_)
{

}

}
