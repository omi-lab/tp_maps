#include "tp_maps/layers/PostOutlineLayer.h"
#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostOutlineLayer::PostOutlineLayer():
  PostLayer({tp_maps::RenderPass::Custom, postOutlineShaderSID()})
{

}

//##################################################################################################
PostShader* PostOutlineLayer::makeShader()
{
  return map()->getShader<PostOutlineShader>();
}

}
