#include "tp_maps/layers/PostGammaLayer.h"
#include "tp_maps/shaders/PostGammaShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostGammaLayer::PostGammaLayer():
  PostLayer({tp_maps::RenderPass::Custom, postGammaShaderSID()})
{

}

//##################################################################################################
PostShader* PostGammaLayer::makeShader()
{
  return map()->getShader<PostGammaShader>();
}

}
