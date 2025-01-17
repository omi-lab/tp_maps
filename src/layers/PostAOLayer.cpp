#include "tp_maps/layers/PostAOLayer.h"
#include "tp_maps/shaders/PostAOShader.h"
#include "tp_maps/shaders/PostAOMergeShader.h"
#include "tp_maps/shaders/PostBasicBlurShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Errors.h"
#include "tp_maps/subsystems/open_gl/OpenGLBuffers.h" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
struct PostAOLayer::Private
{
  Q* q;
  PostAOParameters parameters;

  tp_utils::StringID ssaoPass1{"SSAO 1"};
  tp_utils::StringID ssaoPass2{"SSAO 2"};
  tp_utils::StringID ssaoPass3{"SSAO 3"};

  RenderPass customRenderPass1{tp_maps::RenderPass::Custom, ssaoPass1};
  RenderPass customRenderPass2{tp_maps::RenderPass::Custom, ssaoPass2};
  RenderPass customRenderPass3{tp_maps::RenderPass::Custom, ssaoPass3};

  OpenGLFBO ssaoFbo;
  OpenGLFBO blurFbo;

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
    {
      q->map()->deleteShader(PostAOShader::name());
      q->map()->deleteShader(PostAOMergeShader::name());
    }
  }
};

//##################################################################################################
PostAOLayer::PostAOLayer():
  PostLayer({tp_maps::RenderPass::Custom, ambientOcclusionShaderSID()}),
  d(new Private(this))
{

}

//##################################################################################################
PostAOLayer::~PostAOLayer()
{
  if(map())
  {
    map()->buffers().deleteBuffer(d->ssaoFbo);
    map()->buffers().deleteBuffer(d->blurFbo);
  }
  delete d;
}

//##################################################################################################
const PostAOParameters& PostAOLayer::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void PostAOLayer::setParameters(const PostAOParameters& parameters)
{
  d->parameters = parameters;

  setBypass(!parameters.enabled);

  d->recompileShaders();
}

//##################################################################################################
PostShader* PostAOLayer::makeShader()
{
  return map()->getShader<PostAOShader>(d->parameters);
}

//##################################################################################################
void PostAOLayer::addRenderPasses(std::vector<RenderPass>& renderPasses)
{
  if(bypass())
    return;

  renderPasses.emplace_back(RenderPass::SwapToFBO, ambientOcclusionShaderSID());
  renderPasses.emplace_back(d->customRenderPass1);
  renderPasses.emplace_back(d->customRenderPass2);
  renderPasses.emplace_back(d->customRenderPass3);
}

//##################################################################################################
void PostAOLayer::render(tp_maps::RenderInfo& renderInfo)
{
  if(renderInfo.pass == d->customRenderPass1) //----------------------------------------------------
  {
    d->ssaoFbo.name = "ssao";
    if(!map()->buffers().prepareBuffer(d->ssaoFbo,
                                       map()->width(),
                                       map()->height(),
                                       CreateColorBuffer::Yes,
                                       Multisample::No,
                                       HDR::No,
                                       ExtendedFBO::No,
                                       true))
    {
      Errors::printOpenGLError("SSAO FBO creation failed!");
      return;
    }

    auto ambientOcclusionShader = map()->getShader<PostAOShader>(d->parameters);
    tp_maps::PostLayer::renderToFbo(ambientOcclusionShader, d->ssaoFbo );
  }

  else if(renderInfo.pass == d->customRenderPass2) //-----------------------------------------------
  {
    d->blurFbo.name = "ssaoBlurred";
   if(!map()->buffers().prepareBuffer(d->blurFbo,
                                      map()->width(),
                                      map()->height(),
                                      CreateColorBuffer::Yes,
                                      Multisample::No,
                                      HDR::No,
                                      ExtendedFBO::No,
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
    auto mergeAmbientOcclusionShader = map()->getShader<PostAOMergeShader>(d->parameters);

    auto bindAdditionalTextures = [&]()
    {
      mergeAmbientOcclusionShader->setSSAOTexture(d->blurFbo.textureID);
    };

    tp_maps::PostLayer::renderWithShader(mergeAmbientOcclusionShader, bindAdditionalTextures);
  }

}

//##################################################################################################
void PostAOLayer::invalidateBuffers()
{
  map()->buffers().invalidateBuffer( d->ssaoFbo );
  map()->buffers().invalidateBuffer( d->blurFbo );

  PostLayer::invalidateBuffers();
}


}
