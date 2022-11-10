#include "tp_maps/layers/AmbientOcclusionLayer.h"
#include "tp_maps/shaders/AmbientOcclusionShader.h"
#include "tp_maps/shaders/PostBasicBlurShader.h"
#include "tp_maps/shaders/MergeAmbientOcclusionShader.h"
#include "tp_maps/Map.h"

#include "tp_maps/Buffers.h"
#include "tp_maps/Errors.h"

namespace tp_maps
{

//##################################################################################################
struct AmbientOcclusionLayer::Private
{
  AmbientOcclusionLayer* q;
  AmbientOcclusionParameters parameters;

  tp_utils::StringID ssaoPass1{"SSAO 1"};
  tp_utils::StringID ssaoPass2{"SSAO 2"};
  tp_utils::StringID ssaoPass3{"SSAO 3"};

  RenderPass customRenderPass1{tp_maps::RenderPass::Custom, ssaoPass1};
  RenderPass customRenderPass2{tp_maps::RenderPass::Custom, ssaoPass2};
  RenderPass customRenderPass3{tp_maps::RenderPass::Custom, ssaoPass3};

  FBO ssaoFbo;
  FBO blurFbo;

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
    {
      q->map()->deleteShader(AmbientOcclusionShader::name());
      q->map()->deleteShader(MergeAmbientOcclusionShader::name());
    }
  }

  //################################################################################################
  Private(AmbientOcclusionLayer* q_):
    q(q_)
  {

  }
};

//##################################################################################################
AmbientOcclusionLayer::AmbientOcclusionLayer():
  PostLayer({tp_maps::RenderPass::Custom, ambientOcclusionShaderSID()}),
  d(new Private(this))
{

}

//##################################################################################################
AmbientOcclusionLayer::~AmbientOcclusionLayer()
{
  if(map())
  {
    map()->buffers().deleteBuffer( d->ssaoFbo );
    map()->buffers().deleteBuffer( d->blurFbo );
  }
  delete d;
}

//##################################################################################################
const AmbientOcclusionParameters& AmbientOcclusionLayer::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void AmbientOcclusionLayer::setParameters(const AmbientOcclusionParameters& parameters)
{
  d->parameters = parameters;

  setBypass(!parameters.enabled);

  d->recompileShaders();
}

//##################################################################################################
PostShader* AmbientOcclusionLayer::makeShader()
{
  return map()->getShader<AmbientOcclusionShader>(d->parameters);
}

//##################################################################################################
void AmbientOcclusionLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::SwapToFBO, ambientOcclusionShaderSID());
  renderPasses.emplace_back(d->customRenderPass1);
  renderPasses.emplace_back(d->customRenderPass2);
  renderPasses.emplace_back(d->customRenderPass3);
}

//##################################################################################################
void AmbientOcclusionLayer::render(tp_maps::RenderInfo& renderInfo)
{
  if(renderInfo.pass == d->customRenderPass1) //----------------------------------------------------
  {
    if(!map()->buffers().prepareBuffer(d->ssaoFbo,
                                       map()->width(),
                                       map()->height(),
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       1,
                                       0,
                                       true))
    {
      Errors::printOpenGLError("SSAO FBO creation failed!");
      return;
    }

    auto ambientOcclusionShader = map()->getShader<AmbientOcclusionShader>(d->parameters);
    tp_maps::PostLayer::renderToFbo(ambientOcclusionShader, d->ssaoFbo );
  }

  else if(renderInfo.pass == d->customRenderPass2) //-----------------------------------------------
  {
    if(!map()->buffers().prepareBuffer(d->blurFbo,
                                       map()->width(),
                                       map()->height(),
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       1,
                                       0,
                                       true))
    {
      Errors::printOpenGLError("SSAO Blur FBO creation failed!");
      return;
    }

    auto postBasicBlurShader = map()->getShader<PostBasicBlurShader>();
    tp_maps::PostLayer::renderToFbo(postBasicBlurShader, d->blurFbo, d->ssaoFbo.textureID );

  }

  else if(renderInfo.pass == d->customRenderPass3) //-----------------------------------------------
  {
    auto mergeAmbientOcclusionShader = map()->getShader<MergeAmbientOcclusionShader>(d->parameters);

    auto bindAdditionalTextures = [&]()
    {
      mergeAmbientOcclusionShader->setSSAOTexture(d->blurFbo.textureID);
    };

    tp_maps::PostLayer::renderWithShader(mergeAmbientOcclusionShader, bindAdditionalTextures);
  }

}

//##################################################################################################
void AmbientOcclusionLayer::invalidateBuffers()
{
  map()->buffers().invalidateBuffer( d->ssaoFbo );
  map()->buffers().invalidateBuffer( d->blurFbo );

  PostLayer::invalidateBuffers();
}


}
