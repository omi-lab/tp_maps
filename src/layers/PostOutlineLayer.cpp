#include "tp_maps/layers/PostOutlineLayer.h"
#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
struct PostOutlineLayer::Private
{
  RenderPass customRenderPass1;

  //################################################################################################
  Private(RenderPass customRenderPass1_):
    customRenderPass1(customRenderPass1_)
  {

  }
};

//##################################################################################################
PostOutlineLayer::PostOutlineLayer(Map* map, RenderPass customRenderPass1, RenderPass customRenderPass2):
  PostLayer(map, customRenderPass2),
  d(new Private(customRenderPass1))
{

}

//##################################################################################################
PostOutlineLayer::~PostOutlineLayer()
{
  delete d;
}

//##################################################################################################
RenderPass PostOutlineLayer::customRenderPass1() const
{
  return d->customRenderPass1;
}

//##################################################################################################
RenderPass PostOutlineLayer::customRenderPass2() const
{
  return defaultRenderPass();
}

//##################################################################################################
PostShader* PostOutlineLayer::makeShader()
{
  return map()->getShader<PostOutlineShader>();
}

}
