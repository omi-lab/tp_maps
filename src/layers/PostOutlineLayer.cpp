#include "tp_maps/layers/PostOutlineLayer.h"
#include "tp_maps/shaders/PostBasicBlurShader.h"
#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Errors.h"
#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
struct PostOutlineLayer::Private
{
  Q* q;

  tp_utils::StringID outlineDraw  { "outlineDraw" };
  RenderPass outlineDrawPass { tp_maps::RenderPass::Custom, outlineDraw  };

  tp_utils::StringID outlineBlur  { "outlineBlur" };
  RenderPass outlineBlurPass { tp_maps::RenderPass::Custom, outlineBlur  };

  tp_utils::StringID outlineMerge  { "outlineMerge" };
  RenderPass outlineMergePass { tp_maps::RenderPass::Custom, outlineMerge  };

  //################################################################################################
  Private(Q* q_): q(q_) {}
};

//##################################################################################################
PostOutlineLayer::PostOutlineLayer()
  : PostLayer(RenderPass())
  , d(new Private(this))
{
  setBypass(false);
}

//##################################################################################################
PostOutlineLayer::~PostOutlineLayer()
{
  delete d;
}

//##################################################################################################
PostShader* PostOutlineLayer::makeShader()
{
  return map()->getShader<PostOutlineShader>();
}

//##################################################################################################
void PostOutlineLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;


  renderPasses.emplace_back(RenderPass::PushFBOs);
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineDraw);
  renderPasses.emplace_back(d->outlineDrawPass);
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineBlur);
  renderPasses.emplace_back(d->outlineBlurPass);
  renderPasses.emplace_back(RenderPass::PopFBOs);
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineMerge);
  renderPasses.emplace_back(d->outlineMergePass);
}

//##################################################################################################
void PostOutlineLayer::render(tp_maps::RenderInfo& renderInfo)
{
  if (renderInfo.pass == d->outlineDrawPass)
  {
    // d->outlineDrawFBO.name = d->outlineDraw;
    // if(!map()->buffers().prepareBuffer(d->outlineDrawFBO,
    //                               map()->width()  * 1.0f,
    //                               map()->height() * 1.0f,
    //                               CreateColorBuffer::Yes,
    //                               Multisample::No,
    //                               HDR::No,
    //                               ExtendedFBO::No,
    //                               true))
    // {
    //   Errors::printOpenGLError("outlineDrawFBO creation failed");
    //   return;
    // }

    auto s = map()->getShader<PostOutlineShader>();
    s->mode = 0;
    s->outlineTexID = 0;
    tp_maps::PostLayer::renderWithShader(s);
    // tp_maps::PostLayer::renderToFbo(s, d->outlineDrawFBO);
    return;
  }

  if (renderInfo.pass == d->outlineBlurPass)
  {
    // d->outlineBlurFBO.name = d->outlineBlur;
    // if(!map()->buffers().prepareBuffer(d->outlineBlurFBO,
    //                                     map()->width()  * 1.0f,
    //                                     map()->height() * 1.0f,
    //                                     CreateColorBuffer::Yes,
    //                                     Multisample::No,
    //                                     HDR::No,
    //                                     ExtendedFBO::No,
    //                                     true))
    // {
    //   Errors::printOpenGLError("outlineBlurFBO creation failed");
    //   return;
    // }

    auto s = map()->getShader<PostBasicBlurShader>();
    tp_maps::PostLayer::renderWithShader(s);
    // tp_maps::PostLayer::renderToFbo(s, d->outlineBlurFBO, d->outlineDrawFBO.textureID);
    return;
  }

  auto *outlineBlurFBO = map()->intermediateBuffer(d->outlineBlur);
  if (renderInfo.pass == d->outlineMergePass && outlineBlurFBO)
  {
    auto s = map()->getShader<PostOutlineShader>();
    s->mode = 1;
    s->outlineTexID = outlineBlurFBO->textureID;
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }
}

}
