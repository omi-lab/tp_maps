#include "tp_maps/layers/PostBlurAndTintLayer.h"
#include "tp_maps/shaders/PostBlurAndTintShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostBlurAndTintLayer::PostBlurAndTintLayer():
  PostLayer({tp_maps::RenderPass::Custom, postBlurAndTintShaderSID()})
{
  setBypass(true);
}

//##################################################################################################
PostShader* PostBlurAndTintLayer::makeShader()
{
  return map()->getShader<PostBlurAndTintShader>();
}


//##################################################################################################
void PostBlurAndTintLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::SwapToFBO, postBlurAndTintShaderSID());
  renderPasses.emplace_back(defaultRenderPass());
}

}
