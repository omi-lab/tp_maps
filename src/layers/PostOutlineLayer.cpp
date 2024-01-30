#include "tp_maps/layers/PostOutlineLayer.h"
#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostOutlineLayer::PostOutlineLayer(size_t stageMask, size_t stageUpdate):
  PostSelectionLayer({tp_maps::RenderPass::Custom, postOutlineShaderSID()}, stageMask, stageUpdate)
{

}

//##################################################################################################
PostShader* PostOutlineLayer::makeShader()
{
  return map()->getShader<PostOutlineShader>();
}

}
