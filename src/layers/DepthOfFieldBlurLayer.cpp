#include "tp_maps/layers/DepthOfFieldBlurLayer.h"


#include "tp_maps/shaders/DepthOfFieldBlurShader.h"
#include "tp_maps/shaders/CalculateFocusShader.h"
#include "tp_maps/shaders/DownsampleShader.h"
#include "tp_maps/shaders/MergeDofShader.h"
#include "tp_maps/shaders/PassThroughShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{
//##################################################################################################
struct DepthOfFieldBlurLayer::Private
{
  DepthOfFieldBlurLayer* q;
  DepthOfFieldShaderParameters parameters;

  PostShader* calculateFocusShader;
  MergeDofShader* mergeDofShader;
  PostShader* downsampleShader;
  PostShader* passThroughShader;

  tp_maps::RenderPass customRenderPass1;
  tp_maps::RenderPass customRenderPass2;
  tp_maps::RenderPass customRenderPass3;
  tp_maps::RenderPass customRenderPass4;
  tp_maps::RenderPass customRenderPass5;
  tp_maps::RenderPass customRenderPass6;

  int downsampleFactor{4};

  FBO downsampleFbo;
  FBO focusCalcFbo;
  FBO downsampledFocusCalcFbo;

  //################################################################################################
  void recompileShaders()
  {
    if(q->map())
    {
      q->map()->deleteShader(CalculateFocusShader::name());
      q->map()->deleteShader(MergeDofShader::name());
      q->map()->deleteShader(DepthOfFieldBlurShader::name());
    }
  }

  //################################################################################################
  Private(DepthOfFieldBlurLayer* q_, tp_maps::RenderPass customRenderPass1_, tp_maps::RenderPass customRenderPass2_, tp_maps::RenderPass customRenderPass3_, tp_maps::RenderPass customRenderPass4_, tp_maps::RenderPass customRenderPass5_, tp_maps::RenderPass customRenderPass6_  ):
    q(q_),
    customRenderPass1(customRenderPass1_),
    customRenderPass2(customRenderPass2_),
    customRenderPass3(customRenderPass3_),
    customRenderPass4(customRenderPass4_),
    customRenderPass5(customRenderPass5_),
    customRenderPass6(customRenderPass6_)
  {

  }

  //################################################################################################
  ~Private()
  {
    q->map()->deleteShader(CalculateFocusShader::name());
    q->map()->deleteShader(MergeDofShader::name());
    q->map()->deleteShader(DownsampleShader::name());
    q->map()->deleteShader(DepthOfFieldBlurShader::name());
  }
};

//##################################################################################################
DepthOfFieldBlurLayer::DepthOfFieldBlurLayer(Map* map,
                                             tp_maps::RenderPass customRenderPass1,
                                             tp_maps::RenderPass customRenderPass2,
                                             tp_maps::RenderPass customRenderPass3,
                                             tp_maps::RenderPass customRenderPass4,
                                             tp_maps::RenderPass customRenderPass5,
                                             tp_maps::RenderPass customRenderPass6):
  PostLayer(map, customRenderPass2),
  d(new Private(this, customRenderPass1, customRenderPass2, customRenderPass3, customRenderPass4, customRenderPass5, customRenderPass6))
{
  setBypass(false);

  //  map->setCustomRenderPass(customRenderPass2, [this](RenderInfo&)
  //  {
  //    if( !this->map()->prepareBuffer( d->focusCalcFbo, this->map()->width(), this->map()->height(), CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
  //    {
  //      this->map()->printOpenGLError("Focus calc FBO creation failed!");
  //      return;
  //    }
  //  });

  //  map->setCustomRenderPass(customRenderPass3, [this](RenderInfo&)
  //  {
  //    if( !this->map()->prepareBuffer( d->downsampledFocusCalcFbo, this->map()->width() / d->downsampleFactor, this->map()->height() / d->downsampleFactor, CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
  //    {
  //      this->map()->printOpenGLError("Downsampled focus calc FBO creation failed!");
  //      return;
  //    }
  //  });


  //  map->setCustomRenderPass(customRenderPass5, [this](RenderInfo&)
  //  {
  //    if( !this->map()->prepareBuffer( d->downsampleFbo, this->map()->width() / d->downsampleFactor, this->map()->height() / d->downsampleFactor, CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
  //    {
  //      this->map()->printOpenGLError("Downsample FBO creation failed!");
  //      return;
  //    }
  //  });
}

//##################################################################################################
DepthOfFieldBlurLayer::~DepthOfFieldBlurLayer()
{
  if(map())
  {
    map()->deleteBuffer( d->downsampleFbo );
    map()->deleteBuffer( d->focusCalcFbo );
    map()->deleteBuffer( d->downsampledFocusCalcFbo );
  }
}

//##################################################################################################
const DepthOfFieldShaderParameters& DepthOfFieldBlurLayer::parameters() const
{
  return d->parameters;
}

//##################################################################################################
void DepthOfFieldBlurLayer::setParameters(const DepthOfFieldShaderParameters& parameters)
{
  d->parameters = parameters;
  d->recompileShaders();
}

//##################################################################################################
float DepthOfFieldBlurLayer::calculateFStopDistance( float fStop ) const
{
  float minFStop = 2;
  float maxFStop = 7;
  float fraction = (fStop - minFStop) / (maxFStop - minFStop);

  float minDiffToFocalPlane = 0.05;
  float maxDiffToFocalPlane = 1.5;
  float distance = minDiffToFocalPlane + (maxDiffToFocalPlane - minDiffToFocalPlane) * fraction;

  return distance;
}

//##################################################################################################
PostShader* DepthOfFieldBlurLayer::makeShader()
{
  // Compile all the other shaders
  d->calculateFocusShader = map()->getShader<CalculateFocusShader>([&](Map* m, tp_maps::OpenGLProfile p)
  {
      return new CalculateFocusShader(m, p, d->parameters);
});

  d->downsampleShader = map()->getShader<DownsampleShader>([&](Map* m, tp_maps::OpenGLProfile p)
  {
      return new DownsampleShader(m, p, d->parameters);
});

  d->mergeDofShader = map()->getShader<MergeDofShader>([&](Map* m, tp_maps::OpenGLProfile p)
  {
      return new MergeDofShader(m, p, d->parameters);
});

  d->passThroughShader = map()->getShader<PassThroughShader>();

  // but return this one
  return map()->getShader<DepthOfFieldBlurShader>([&](Map* m, tp_maps::OpenGLProfile p)
  {
    return new DepthOfFieldBlurShader(m, p, d->parameters);
  });
}

//##################################################################################################
void DepthOfFieldBlurLayer::render(tp_maps::RenderInfo& renderInfo)
{
  // Get all required shaders - makeShader will compile them all
  makeShader();

  auto calculateFocusShader = d->calculateFocusShader;
  auto downsampleShader = d->downsampleShader;
  auto mergeDofShader = d->mergeDofShader;

  auto passThroughShader = d->passThroughShader;

  if(renderInfo.pass == d->customRenderPass1)
  {
    tp_maps::PostLayer::renderWithShader( renderInfo, passThroughShader ); // Just rendering texture to FBO
  }

  if(renderInfo.pass == d->customRenderPass2)
  {
    if(!map()->prepareBuffer( d->focusCalcFbo, map()->width(), map()->height(), CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
    {
      map()->printOpenGLError("Focus calc FBO creation failed!");
      return;
    }
    tp_maps::PostLayer::renderToFbo( renderInfo, calculateFocusShader, d->focusCalcFbo ); // New fbo for focus texture ( using R channel )
  }

  if(renderInfo.pass == d->customRenderPass3)
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->prepareBuffer(d->downsampledFocusCalcFbo, width, height, CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
    {
      map()->printOpenGLError("Downsampled focus calc FBO creation failed!");
      return;
    }
    tp_maps::PostLayer::renderToFbo( renderInfo, calculateFocusShader, d->downsampledFocusCalcFbo, d->focusCalcFbo.textureID ); // Downsample the focus FBO
  }

  if(renderInfo.pass == d->customRenderPass4)
  {
    tp_maps::PostLayer::renderWithShader( renderInfo, passThroughShader ); // Just rendering texture to FBO
  }

  if( renderInfo.pass == d->customRenderPass5 )
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->prepareBuffer( d->downsampleFbo, width, height, CreateColorBuffer::Yes, Multisample::No, HDR::No, ExtendedFBO::No, 1, 0, true ) )
    {
      map()->printOpenGLError("Downsample FBO creation failed!");
      return;
    }
    tp_maps::PostLayer::renderToFbo( renderInfo, downsampleShader, d->downsampleFbo ); // Downsample the regular color FBO
  }

  if( renderInfo.pass == d->customRenderPass6 )
  {
    auto bindAdditionalTextures = [this]() {
      d->mergeDofShader->setDownsampledTexture( d->downsampleFbo.textureID );
      d->mergeDofShader->setFocusTexture( d->focusCalcFbo.textureID );
      d->mergeDofShader->setDownsampledFocusTexture( d->downsampledFocusCalcFbo.textureID );
    };

    tp_maps::PostLayer::renderWithShader( renderInfo, mergeDofShader, bindAdditionalTextures );
  }
}

//##################################################################################################
void DepthOfFieldBlurLayer::invalidateBuffers()
{
  map()->invalidateBuffer( d->downsampleFbo );
  map()->invalidateBuffer( d->focusCalcFbo );
  map()->invalidateBuffer( d->downsampledFocusCalcFbo );

  Layer::invalidateBuffers();
}

}
