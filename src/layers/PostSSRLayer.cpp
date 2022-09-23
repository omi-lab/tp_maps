#include "tp_maps/layers/PostSSRLayer.h"
#include "tp_maps/shaders/PostSSRShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostSSRLayer::PostSSRLayer(RenderPass::RenderPassType customRenderPass):
  PostLayer(customRenderPass)
{

}

//##################################################################################################
PostShader* PostSSRLayer::makeShader()
{
  return map()->getShader<PostSSRShader>();
}

}
