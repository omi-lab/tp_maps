#include "tp_maps/layers/PostDoFLayer.h"
#include "tp_maps/Controller.h"
#include "tp_maps/Buffers.h"
#include "tp_maps/Errors.h"

#include "tp_maps/shaders/PostDoFBlurShader.h"
#include "tp_maps/shaders/PostDoFCalculateFocusShader.h"
#include "tp_maps/shaders/PostDoFDownsampleShader.h"
#include "tp_maps/shaders/PostDoFMergeShader.h"
#include "tp_maps/shaders/PassThroughShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
struct PostDoFLayer::Private
{
  PostDoFLayer* q;
  PostDoFParameters parameters;

  tp_utils::StringID dofPass1{"DoF 1"};
  tp_utils::StringID dofPass2{"DoF 2"};
  tp_utils::StringID dofPass3{"DoF 3"};
  tp_utils::StringID dofPass4{"DoF 4"};
  tp_utils::StringID dofPass5{"DoF 5"};
  tp_utils::StringID dofPass6{"DoF 6"};

  RenderPass customRenderPass1{tp_maps::RenderPass::Custom, dofPass1};
  RenderPass customRenderPass2{tp_maps::RenderPass::Custom, dofPass2};
  RenderPass customRenderPass3{tp_maps::RenderPass::Custom, dofPass3};
  RenderPass customRenderPass4{tp_maps::RenderPass::Custom, dofPass4};
  RenderPass customRenderPass5{tp_maps::RenderPass::Custom, dofPass5};
  RenderPass customRenderPass6{tp_maps::RenderPass::Custom, dofPass6};

  int downsampleFactor{4};

  FBO downsampleFbo;
  FBO focusCalcFbo;
  FBO downsampledFocusCalcFbo;

  //################################################################################################
  Private(PostDoFLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
    {
      q->map()->deleteShader(PostDoFCalculateFocusShader::name());
      q->map()->deleteShader(PostDoFMergeShader::name());
      q->map()->deleteShader(PostDoFDownsampleShader::name());
      q->map()->deleteShader(PostDoFBlurShader::name());
    }
  }
};

//##################################################################################################
PostDoFLayer::PostDoFLayer():
  PostLayer(RenderPass()),
  d(new Private(this))
{
  setBypass(false);
}

//##################################################################################################
PostDoFLayer::~PostDoFLayer()
{
  if(map())
  {
    map()->buffers().deleteBuffer(d->downsampleFbo);
    map()->buffers().deleteBuffer(d->focusCalcFbo);
    map()->buffers().deleteBuffer(d->downsampledFocusCalcFbo);
  }

  delete d;
}

//##################################################################################################
const PostDoFParameters& PostDoFLayer::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void PostDoFLayer::setParameters(const PostDoFParameters& parameters)
{
  if(!parameters.fuzzyEquals(d->parameters))
  {
    d->parameters = parameters;
    setBypass(!parameters.enabled);
    d->recompileShaders();
  }
}

//##################################################################################################
float PostDoFLayer::calculateFStopDistance(float fStop) const
{
  float minFStop = 2.0f;
  float maxFStop = 7.0f;
  float fraction = (fStop - minFStop) / (maxFStop - minFStop);

  float minDiffToFocalPlane = 0.05f;
  float maxDiffToFocalPlane = 1.5f;
  float distance = minDiffToFocalPlane + (maxDiffToFocalPlane - minDiffToFocalPlane) * fraction;

  return distance;
}

//##################################################################################################
PostShader* PostDoFLayer::makeShader()
{
  return map()->getShader<PostDoFBlurShader>(d->parameters);
}

//##################################################################################################
void PostDoFLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::SwapToFBO, mergeDofShaderSID());
  renderPasses.emplace_back(d->customRenderPass1);
  renderPasses.emplace_back(d->customRenderPass2);
  renderPasses.emplace_back(d->customRenderPass3);
  renderPasses.emplace_back(RenderPass::SwapToFBO, depthOfFieldBlurShaderSID());
  renderPasses.emplace_back(d->customRenderPass4);
  renderPasses.emplace_back(d->customRenderPass5);
  renderPasses.emplace_back(RenderPass::SwapToFBO, mergeDofShaderSID());
  renderPasses.emplace_back(d->customRenderPass6);
}

//##################################################################################################
void PostDoFLayer::render(tp_maps::RenderInfo& renderInfo)
{
  auto setNearAndFar=[&](auto shader)
  {
    auto nearAndFar = map()->controller()->matrices(tp_maps::defaultSID()).nearAndFar();
    shader->setProjectionNearAndFar(nearAndFar.x, nearAndFar.y);
  };

  if(renderInfo.pass == d->customRenderPass1) //----------------------------------------------------
  {
    auto passThroughShader = map()->getShader<PassThroughShader>();
    tp_maps::PostLayer::renderWithShader(passThroughShader); // Just rendering texture to FBO
  }


  else if(renderInfo.pass == d->customRenderPass2) //-----------------------------------------------
  {
    if(!map()->buffers().prepareBuffer("dofFocus",
                                       d->focusCalcFbo,
                                       map()->width(),
                                       map()->height(),
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       true))
    {
      Errors::printOpenGLError("Focus calc FBO creation failed!");
      return;
    }

    // New fbo for focus texture ( using R channel )
    auto calculateFocusShader = map()->getShader<PostDoFCalculateFocusShader>(d->parameters);
    setNearAndFar(calculateFocusShader);
    tp_maps::PostLayer::renderToFbo(calculateFocusShader, d->focusCalcFbo);
  }


  else if(renderInfo.pass == d->customRenderPass3) //-----------------------------------------------
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->buffers().prepareBuffer("dofFocusDownsampled",
                                       d->downsampledFocusCalcFbo,
                                       width,
                                       height,
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       true))
    {
      Errors::printOpenGLError("Downsampled focus calc FBO creation failed!");
      return;
    }

    // Downsample the focus FBO
    auto calculateFocusShader = map()->getShader<PostDoFCalculateFocusShader>(d->parameters);
    setNearAndFar(calculateFocusShader);
    tp_maps::PostLayer::renderToFbo(calculateFocusShader,
                                    d->downsampledFocusCalcFbo,
                                    d->focusCalcFbo.textureID );
  }


  else if(renderInfo.pass == d->customRenderPass4) //-----------------------------------------------
  {
    auto passThroughShader = map()->getShader<PassThroughShader>();
    tp_maps::PostLayer::renderWithShader(passThroughShader ); // Just rendering texture to FBO
  }


  else if(renderInfo.pass == d->customRenderPass5)
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->buffers().prepareBuffer("dofColorDownsampled",
                                       d->downsampleFbo,
                                       width,
                                       height,
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       true))
    {
      Errors::printOpenGLError("Downsample FBO creation failed!");
      return;
    }

    // Downsample the regular color FBO
    auto downsampleShader = map()->getShader<PostDoFDownsampleShader>(d->parameters);
    setNearAndFar(downsampleShader);
    tp_maps::PostLayer::renderToFbo(downsampleShader, d->downsampleFbo );
  }


  else if(renderInfo.pass == d->customRenderPass6) //-----------------------------------------------
  {
    auto mergeDofShader = map()->getShader<PostDoFMergeShader>(d->parameters);
    setNearAndFar(mergeDofShader);

    auto bindAdditionalTextures = [&]()
    {
      mergeDofShader->setDownsampledTexture(d->downsampleFbo.textureID);
      mergeDofShader->setFocusTexture(d->focusCalcFbo.textureID);
      mergeDofShader->setDownsampledFocusTexture( d->downsampledFocusCalcFbo.textureID);
    };

    tp_maps::PostLayer::renderWithShader(mergeDofShader, bindAdditionalTextures);
  }
}

//##################################################################################################
void PostDoFLayer::invalidateBuffers()
{
  map()->buffers().invalidateBuffer(d->downsampleFbo);
  map()->buffers().invalidateBuffer(d->focusCalcFbo);
  map()->buffers().invalidateBuffer(d->downsampledFocusCalcFbo);

  PostLayer::invalidateBuffers();
}

}
