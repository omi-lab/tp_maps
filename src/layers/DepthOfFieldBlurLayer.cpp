#include "tp_maps/layers/DepthOfFieldBlurLayer.h"

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"
#include "tp_maps/shaders/CalculateFocusShader.h"
#include "tp_maps/shaders/DownsampleShader.h"
#include "tp_maps/shaders/MergeDofShader.h"
#include "tp_maps/shaders/PassThroughShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{
//##################################################################################################
struct DepthOfFieldBlurLayer::Private
{
  DepthOfFieldBlurLayer* q;
  DepthOfFieldShaderParameters parameters;

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
      q->map()->deleteShader(DownsampleShader::name());
      q->map()->deleteShader(DepthOfFieldBlurShader::name());
    }
  }

  //################################################################################################
  Private(DepthOfFieldBlurLayer* q_,
          tp_maps::RenderPass customRenderPass1_,
          tp_maps::RenderPass customRenderPass2_,
          tp_maps::RenderPass customRenderPass3_,
          tp_maps::RenderPass customRenderPass4_,
          tp_maps::RenderPass customRenderPass5_,
          tp_maps::RenderPass customRenderPass6_):
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

  delete d;
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
  return map()->getShader<DepthOfFieldBlurShader>(d->parameters);
}

//##################################################################################################
void DepthOfFieldBlurLayer::render(tp_maps::RenderInfo& renderInfo)
{
  // Get all required shaders - makeShader will compile them all
  makeShader();

  auto calculateFocusShader = map()->getShader<CalculateFocusShader>(d->parameters);
  auto downsampleShader = map()->getShader<DownsampleShader>(d->parameters);
  auto mergeDofShader = map()->getShader<MergeDofShader>(d->parameters);
  auto passThroughShader = map()->getShader<PassThroughShader>();


  if(renderInfo.pass == d->customRenderPass1) //----------------------------------------------------
  {
    tp_maps::PostLayer::renderWithShader(passThroughShader); // Just rendering texture to FBO
  }


  else if(renderInfo.pass == d->customRenderPass2) //-----------------------------------------------
  {
    if(!map()->prepareBuffer(d->focusCalcFbo,
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
      map()->printOpenGLError("Focus calc FBO creation failed!");
      return;
    }

    // New fbo for focus texture ( using R channel )
    tp_maps::PostLayer::renderToFbo(calculateFocusShader, d->focusCalcFbo );
  }


  else if(renderInfo.pass == d->customRenderPass3) //-----------------------------------------------
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->prepareBuffer(d->downsampledFocusCalcFbo,
                             width,
                             height,
                             CreateColorBuffer::Yes,
                             Multisample::No,
                             HDR::No,
                             ExtendedFBO::No,
                             1,
                             0,
                             true))
    {
      map()->printOpenGLError("Downsampled focus calc FBO creation failed!");
      return;
    }

     // Downsample the focus FBO
    tp_maps::PostLayer::renderToFbo(calculateFocusShader,
                                    d->downsampledFocusCalcFbo,
                                    d->focusCalcFbo.textureID );
  }


  else if(renderInfo.pass == d->customRenderPass4) //-----------------------------------------------
  {
    tp_maps::PostLayer::renderWithShader(passThroughShader ); // Just rendering texture to FBO
  }


  else if(renderInfo.pass == d->customRenderPass5)
  {
    size_t width  = std::max(1, map()->width() / d->downsampleFactor);
    size_t height = std::max(1, map()->height() / d->downsampleFactor);

    if(!map()->prepareBuffer(d->downsampleFbo,
                             width,
                             height,
                             CreateColorBuffer::Yes,
                             Multisample::No,
                             HDR::No,
                             ExtendedFBO::No,
                             1,
                             0,
                             true))
    {
      map()->printOpenGLError("Downsample FBO creation failed!");
      return;
    }

    // Downsample the regular color FBO
    tp_maps::PostLayer::renderToFbo(downsampleShader, d->downsampleFbo );
  }


  else if(renderInfo.pass == d->customRenderPass6) //-----------------------------------------------
  {
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
void DepthOfFieldBlurLayer::invalidateBuffers()
{
  map()->invalidateBuffer( d->downsampleFbo );
  map()->invalidateBuffer( d->focusCalcFbo );
  map()->invalidateBuffer( d->downsampledFocusCalcFbo );

  PostLayer::invalidateBuffers();
}

}
