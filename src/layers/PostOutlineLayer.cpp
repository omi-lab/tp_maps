#include "tp_maps/layers/PostOutlineLayer.h"

#include "tp_maps/Map.h"
#include "tp_maps/Errors.h"
#include "tp_utils/DebugUtils.h"

#include "tp_maps/shaders/PostOutlineShader.h"
#include "tp_maps/shaders/PostBasicBlurShader.h"
#include "tp_maps/shaders/PostTwoPassBlurShader.h"

namespace tp_maps
{

//##################################################################################################
struct PostOutlineLayer::Private
{
  Q* q;

  tp_utils::StringID outlineDraw  { "outlineDraw" };
  RenderPass outlineDrawPass { tp_maps::RenderPass::Custom, outlineDraw  };

  tp_utils::StringID outlineBlurH  { "outlineBlurH" };
  RenderPass outlineBlurHPass { tp_maps::RenderPass::Custom, outlineBlurH  };
  tp_utils::StringID outlineBlurV  { "outlineBlurV" };
  RenderPass outlineBlurVPass { tp_maps::RenderPass::Custom, outlineBlurV  };

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
#define POST_OUTLINE_USE_TWO_PASS_BLUR
void PostOutlineLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::PushFBOs);
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineDraw);
  renderPasses.emplace_back(d->outlineDrawPass);
#ifdef POST_OUTLINE_USE_TWO_PASS_BLUR
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineBlurH);
  renderPasses.emplace_back(d->outlineBlurHPass);
#endif
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineBlurV);
  renderPasses.emplace_back(d->outlineBlurVPass);
  renderPasses.emplace_back(RenderPass::PopFBOs);
  renderPasses.emplace_back(RenderPass::SwapToFBO, d->outlineMerge);
  renderPasses.emplace_back(d->outlineMergePass);
}

//##################################################################################################
void PostOutlineLayer::render(tp_maps::RenderInfo& renderInfo)
{
  if (renderInfo.pass == d->outlineDrawPass)
  {
    auto s = map()->getShader<PostOutlineShader>();
    s->mode = 0;
    s->outlineTexID = 0;
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }

#ifdef POST_OUTLINE_USE_TWO_PASS_BLUR
  if (renderInfo.pass == d->outlineBlurHPass)
  {
    auto s = map()->getShader<PostTwoPassBlurShader>();
    s->dirIndex = 0;
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }

  if (renderInfo.pass == d->outlineBlurVPass)
  {
    auto s = map()->getShader<PostTwoPassBlurShader>();
    s->dirIndex = 1;
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }
#else
  if (renderInfo.pass == d->outlineBlurVPass)
  {
    auto s = map()->getShader<PostBasicBlurShader>();
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }
#endif

  auto *outlineBlurVFBO = map()->intermediateBuffer(d->outlineBlurV);
  // assert(outlineBlurVFBO);
  if (renderInfo.pass == d->outlineMergePass && outlineBlurVFBO)
  {
    auto s = map()->getShader<PostOutlineShader>();
    s->mode = 1;
    s->outlineTexID = outlineBlurVFBO->textureID;
    tp_maps::PostLayer::renderWithShader(s);
    return;
  }
}

}
