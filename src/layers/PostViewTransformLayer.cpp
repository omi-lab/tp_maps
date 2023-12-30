#include "tp_maps/layers/PostViewTransformLayer.h"
#include "tp_maps/shaders/PostViewTransformShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostViewTransformLayer::PostViewTransformLayer():
  PostLayer({tp_maps::RenderPass::Custom, postViewTransformShaderSID()})
{

}

//##################################################################################################
PostShader* PostViewTransformLayer::makeShader()
{
  return map()->getShader<PostViewTransformShader>();
}

}
