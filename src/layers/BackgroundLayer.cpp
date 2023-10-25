#include "tp_maps/layers/BackgroundLayer.h"
#include "tp_maps/shaders/BackgroundSkyBoxShader.h"
#include "tp_maps/shaders/BackgroundPatternShader.h"
#include "tp_maps/shaders/BackgroundImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/TexturePool.h"

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

  std::function<glm::mat4()> flatMatrixCallback = []{return glm::mat4(1.0f);};

  //################################################################################################
  Private(TexturePool* texturePool_):
    texturePool(texturePool_)
  {

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
  if(d->textureName != textureName)
  {
    d->textureName = textureName;
    update();
  }
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

    auto shader = map()->getShader<BackgroundSkyBoxShader>();
    if(shader->error())
      return;

    auto matricies = map()->controller()->matrices(defaultSID());

    shader->use(renderInfo.shaderType());
    shader->setTexture(d->texturePool->textureID(d->textureName));
    shader->setMatrix(matricies.v, matricies.p);
    shader->setFrameMatrix(glm::mat4(1.0f));
    shader->setRotationFactor(d->rotationFactor);
    shader->draw();

    break;
  }

  case Mode::TransparentPattern: //-----------------------------------------------------------------
  {
    auto shader = map()->getShader<BackgroundPatternShader>();
    if(shader->error())
      return;

    shader->use(renderInfo.shaderType());
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

    shader->use(renderInfo.shaderType());
    shader->setTexture(d->texturePool->textureID(d->textureName));
    shader->setMatrix(d->flatMatrixCallback());
    shader->setFrameMatrix(glm::mat4(1.0f));
    //shader->setFrameMatrix(map()->controller()->matrices("Mask").p);
    shader->draw();

    break;
  }
  }
}

}
