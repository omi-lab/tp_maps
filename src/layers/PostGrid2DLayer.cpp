#include "tp_maps/layers/PostGrid2DLayer.h"
#include "tp_maps/shaders/PostGrid2DShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
PostGrid2DLayer::PostGrid2DLayer(Map* map, RenderPass renderPass):
  PostLayer(map, renderPass)
{
}

//##################################################################################################
PostShader* PostGrid2DLayer::makeShader()
{
  return map()->getShader<PostGrid2DShader>();
}

}
