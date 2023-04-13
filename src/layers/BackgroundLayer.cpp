#include "tp_maps/layers/BackgroundLayer.h"
#include "tp_maps/shaders/BackgroundShader.h"
#include "tp_maps/shaders/PatternShader.h"
#include "tp_maps/shaders/BackgroundImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/TexturePool.h"

#include "tp_image_utils/ToFloat.h"
#include "tp_image_utils_functions/BlurColorMap.h"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct BackgroundLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::BackgroundLayer::Private");
  TP_NONCOPYABLE(Private);

  Mode mode{Mode::Spherical};

  TexturePool* texturePool;
  tp_utils::StringID textureName;
  float rotationFactor{0.0f};

  float gridSpacing{20.0f};

  std::unordered_map< std::string, tp_utils::StringID > blurTextures;
  size_t skyBoxBlurRadius;
  std::string currentBlurTexture;

  std::function<glm::mat4()> flatMatrixCallback = []{return glm::mat4(1.0f);};

  //################################################################################################
  Private(TexturePool* texturePool_):
    texturePool(texturePool_)
  {

  }

  void prepareBlurTextures()
  {
    if(skyBoxBlurRadius == 1)
      return;

    std::string blurredTextureName = textureName.toString() + "_blur_" + std::to_string(skyBoxBlurRadius);
    currentBlurTexture = blurredTextureName;

    if(blurTextures.find(blurredTextureName) == blurTextures.end())
    {
      texturePool->viewImage( textureName, [&](const tp_image_utils::ColorMap& colorMap)
      {
        tp_image_utils::ColorMapF imageF = tp_image_utils::toFloat(colorMap);

        tp_image_utils_functions::blurColorMap(imageF, skyBoxBlurRadius);

        tp_utils::StringID blurredTextureId = tp_utils::StringID(blurredTextureName);
        texturePool->subscribe( blurredTextureId, tp_image_utils::fromFloat(imageF), false);

        texturePool->setTextureWrapS(blurredTextureId, GL_REPEAT);
        texturePool->setTextureWrapT(blurredTextureId, GL_REPEAT);

        blurTextures[blurredTextureName] = blurredTextureId;
      });
    }
  };

  void deleteBlurTextures()
  {
    for(auto& blurTex : blurTextures) 
      if(blurTex.second.isValid())
        texturePool->unsubscribe(blurTex.second);
    
    blurTextures.clear();
  }
};

//##################################################################################################
BackgroundLayer::BackgroundLayer(TexturePool* texturePool):
  d(new Private(texturePool))
{
  setDefaultRenderPass(tp_maps::RenderPass::Background);
}

//##################################################################################################
BackgroundLayer::~BackgroundLayer()
{
  delete d;
}

//##################################################################################################
BackgroundLayer::Mode BackgroundLayer::mode() const
{
  return d->mode;
}

//##################################################################################################
void BackgroundLayer::setMode(BackgroundLayer::Mode mode)
{
  d->mode = mode;
}

//##################################################################################################
const tp_utils::StringID& BackgroundLayer::textureName() const
{
  return d->textureName;
}

//##################################################################################################
void BackgroundLayer::setTextureName(const tp_utils::StringID& textureName)
{
  d->textureName = textureName;
  update();
}

//##################################################################################################
float BackgroundLayer::rotationFactor() const
{
  return d->rotationFactor;
}

//##################################################################################################
void BackgroundLayer::setRotationFactor(float rotationFactor)
{
  d->rotationFactor = rotationFactor;
  update();
}

//##################################################################################################
float BackgroundLayer::gridSpacing() const
{
  return d->gridSpacing;
}

//##################################################################################################
void BackgroundLayer::setGridSpacing(float gridSpacing)
{
  d->gridSpacing = gridSpacing;
  update();
}

//##################################################################################################
void BackgroundLayer::setFlatMatrixCallback(const std::function<glm::mat4()>& flatMatrixCallback)
{
  d->flatMatrixCallback = flatMatrixCallback;
  update();
}

//##################################################################################################
void BackgroundLayer::setSkyboxBlurRadius( size_t skyBoxBlurRadius )
{
  if(d->mode == Mode::Flat)
  {
    d->skyBoxBlurRadius = skyBoxBlurRadius;
    update();
  }
}

//##################################################################################################
void BackgroundLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass().type)
    return;

  switch(d->mode)
  {
  case Mode::Spherical: //--------------------------------------------------------------------------
  {
    if(!d->textureName.isValid())
      return;

    auto shader = map()->getShader<BackgroundShader>();
    if(shader->error())
      return;

    auto matricies = map()->controller()->matrices(defaultSID());

    shader->use(ShaderType::RenderExtendedFBO);
    shader->setTexture(d->texturePool->textureID(d->textureName));
    shader->setMatrix(matricies.v, matricies.p);
    shader->setFrameMatrix(glm::mat4(1.0f));
    shader->setRotationFactor(d->rotationFactor);
    shader->draw();

    break;
  }

  case Mode::TransparentPattern: //-----------------------------------------------------------------
  {
    auto shader = map()->getShader<PatternShader>();
    if(shader->error())
      return;

    shader->use(ShaderType::RenderExtendedFBO);
    shader->setFrameMatrix(glm::mat4(1.0f));
    shader->setScreenSizeAndGridSpacing(map()->screenSize(), d->gridSpacing);
    shader->draw();

    break;
  }

  case Mode::Flat: //-------------------------------------------------------------------------------
  {
    auto shader = map()->getShader<BackgroundImageShader>();
    if(shader->error())
      return;

    shader->use(ShaderType::RenderExtendedFBO);

    auto textureToUse = d->textureName;

    if(d->skyBoxBlurRadius > 1)
    {
      d->prepareBlurTextures();

      if(d->blurTextures.find(d->currentBlurTexture) != d->blurTextures.end())
        textureToUse = d->blurTextures[d->currentBlurTexture];
    }

    shader->setTexture(d->texturePool->textureID(textureToUse));
    shader->setMatrix(d->flatMatrixCallback());
    shader->setFrameMatrix(glm::mat4(1.0f));
    //shader->setFrameMatrix(map()->controller()->matrices("Mask").p);
    shader->draw();

    break;
  }
  }
}

//##################################################################################################
void BackgroundLayer::invalidateBuffers()
{
  d->deleteBlurTextures();
}

}
