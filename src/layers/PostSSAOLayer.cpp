#include "tp_maps/layers/PostSSAOLayer.h"
#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostSSAOLayer::PostSSAOLayer(Map* map, RenderPass customRenderPass):
  PostLayer(map, customRenderPass)
{

}

//##################################################################################################
PostShader* PostSSAOLayer::makeShader()
{
  return map()->getShader<PostSSAOShader>();
}

}
