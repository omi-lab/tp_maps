#include "tp_maps/layers/PostBlurAndTintLayer.h"
#include "tp_maps/shaders/PostBlurAndTintShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
PostBlurAndTintLayer::PostBlurAndTintLayer(Map* map, RenderPass customRenderPass):
  PostLayer(map, customRenderPass)
{
  setBypass(true);
}

//##################################################################################################
PostShader* PostBlurAndTintLayer::makeShader()
{
  return map()->getShader<PostBlurAndTintShader>();
}

}
