#include "tp_maps/layers/PostSSRLayer.h"
#include "tp_maps/shaders/PostSSRShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostSSRLayer::PostSSRLayer(Map* map, RenderPass customRenderPass):
  PostLayer(map, customRenderPass)
{

}

//##################################################################################################
PostShader* PostSSRLayer::makeShader()
{
  return map()->getShader<PostSSRShader>();
}

}
