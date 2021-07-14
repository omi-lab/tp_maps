#include "tp_maps/layers/BackgroundLayer.h"
#include "tp_maps/shaders/BackgroundShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/TexturePool.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct BackgroundLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::BackgroundLayer::Private");
  TP_NONCOPYABLE(Private);

  TexturePool* texturePool;
  tp_utils::StringID textureName;
  float rotationFactor{0.0f};

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
void BackgroundLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass() || !d->textureName.isValid())
    return;

  auto shader = map()->getShader<BackgroundShader>();

  if(shader->error())
    return;

  auto matricies = map()->controller()->matrices(defaultSID());

  shader->use(ShaderType::RenderExtendedFBO);
  shader->setTexture(d->texturePool->textureID(d->textureName));
  shader->setMatrix(matricies.v, matricies.p);
  shader->setRotationFactor(d->rotationFactor);
  shader->draw();
}

}
