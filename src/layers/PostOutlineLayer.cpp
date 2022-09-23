#include "tp_maps/layers/PostOutlineLayer.h"
#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostOutlineLayer::PostOutlineLayer(size_t stage):
  PostSelectionLayer({tp_maps::RenderPass::Custom, postOutlineShaderSID()}, stage)
{

}

//##################################################################################################
PostShader* PostOutlineLayer::makeShader()
{
  return map()->getShader<PostOutlineShader>();
}

}
