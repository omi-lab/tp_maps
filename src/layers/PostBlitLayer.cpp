#include "tp_maps/layers/PostBlitLayer.h"
#include "tp_maps/shaders/PostBlitShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostBlitLayer::PostBlitLayer(Map* map, RenderPass customRenderPass):
  PostLayer(map, customRenderPass)
{

}

//##################################################################################################
PostShader* PostBlitLayer::makeShader()
{
  return map()->getShader<PostBlitShader>();
}

}
