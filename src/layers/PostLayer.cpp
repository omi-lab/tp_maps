#include "tp_maps/layers/PostLayer.h"
#include "tp_maps/shaders/PostShader.h"
#include "tp_maps/shaders/PostBlitShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct PostLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::PostLayer::Private");
  TP_NONCOPYABLE(Private);

  PostLayer* q;

  bool bypass{false};

  FullScreenShader::Object* rectangleObject{nullptr};
  FullScreenShader::Object* frameObject{nullptr};
  tp_utils::StringID frameCoordinateSystem;

  bool rectangle{true};
  glm::vec2 holeSize{0.5f, 0.5f};
  glm::vec2 size{1.0f, 1.0f};

  bool blitRectangle{false};
  bool blitFrame{false};

  //################################################################################################
  Private(PostLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  void freeObject()
  {
    delete rectangleObject;
    delete frameObject;

    rectangleObject = nullptr;
    frameObject = nullptr;
  }
};

//##################################################################################################
PostLayer::PostLayer(Map* map, RenderPass customRenderPass):
  d(new Private(this))
{
  setDefaultRenderPass(customRenderPass);
  map->setCustomRenderPass(customRenderPass, [](RenderInfo&)
  {
    glDepthMask(false);
    glDisable(GL_DEPTH_TEST);
  },[](RenderInfo&)
  {

  });
}

//##################################################################################################
PostLayer::~PostLayer()
{
  d->freeObject();
  delete d;
}

//##################################################################################################
void PostLayer::setFrameCoordinateSystem(const tp_utils::StringID& frameCoordinateSystem)
{
  d->frameCoordinateSystem = frameCoordinateSystem;
}

//##################################################################################################
bool PostLayer::bypass() const
{
  return d->bypass;
}

//##################################################################################################
void PostLayer::setBypass(bool bypass)
{
  d->bypass = bypass;
  update();
}

//##################################################################################################
void PostLayer::setRectangle(const glm::vec2& size)
{
  d->rectangle = true;
  d->size = size;
  d->freeObject();
}

//##################################################################################################
void PostLayer::setFrame(const glm::vec2& holeSize, const glm::vec2& size)
{
  d->rectangle = false;
  d->holeSize = holeSize;
  d->size = size;
  d->freeObject();
}

//##################################################################################################
void PostLayer::setBlit(bool blitRectangle, bool blitFrame)
{
  d->blitRectangle = blitRectangle;
  d->blitFrame = blitFrame;
}

//##################################################################################################
void PostLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass())
    return;


  // Blit shader stuff
  {
    auto shader = map()->getShader<PostBlitShader>();

    if(!d->rectangleObject)
    {
      d->rectangleObject = shader->makeRectangleObject(d->size);
      d->frameObject = shader->makeFrameObject(d->holeSize, d->size);
    }

    if(shader->error())
      return;

    shader->use(ShaderType::RenderExtendedFBO);
    shader->setReadFBO(map()->currentReadFBO());
    shader->setFrameMatrix(map()->controller()->matrices(d->frameCoordinateSystem).p);
    shader->setProjectionMatrix(map()->controller()->matrices(coordinateSystem()).p);

    if(d->blitRectangle)
      shader->draw(*d->rectangleObject);

    if(d->blitFrame)
      shader->draw(*d->frameObject);
  }

  // Post shader stuff
  {
    auto shader = d->bypass?static_cast<PostShader*>(map()->getShader<PostBlitShader>()):makeShader();

    if(shader->error())
      return;

    shader->use(ShaderType::RenderExtendedFBO);
    shader->setReadFBO(map()->currentReadFBO());
    shader->setFrameMatrix(map()->controller()->matrices(d->frameCoordinateSystem).p);
    shader->setProjectionMatrix(map()->controller()->matrices(coordinateSystem()).p);

    if(d->rectangle)
      shader->draw(*d->rectangleObject);
    else
      shader->draw(*d->frameObject);
  }
}

//##################################################################################################
void PostLayer::invalidateBuffers()
{
  delete d->rectangleObject;
  delete d->frameObject;

  d->rectangleObject = nullptr;
  d->frameObject = nullptr;

  Layer::invalidateBuffers();
}


}
