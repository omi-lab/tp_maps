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

  FullScreenShader::Object object;

  bool rectangle{true};
  glm::vec2 holeSize{0.5f, 0.5f};
  glm::vec2 size{1.0f, 1.0f};

  //################################################################################################
  Private(PostLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  void freeObject()
  {
    if(q->map())
    {
      q->map()->makeCurrent();
      FullScreenShader::freeObject(object);
    }
  }
};

//##################################################################################################
PostLayer::PostLayer(Map* map, RenderPass customRenderPass):
  d(new Private(this))
{
  setDefaultRenderPass(customRenderPass);
  map->setCustomRenderPass(customRenderPass, [](RenderInfo&)
  {
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
void PostLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass())
    return;

  auto shader = d->bypass?static_cast<PostShader*>(map()->getShader<PostBlitShader>()):makeShader();

  if(shader->error())
    return;

  if(d->object.size<1)
  {
    if(d->rectangle)
      FullScreenShader::makeRectangleObject(d->object, d->size);
    else
      FullScreenShader::makeFrameObject(d->object, d->holeSize, d->size);
  }

  shader->use(ShaderType::RenderExtendedFBO);
  shader->setReadFBO(map()->currentReadFBO());
  shader->setProjectionMatrix(map()->controller()->matrices(coordinateSystem()).p);
  shader->draw();
}


//##################################################################################################
void PostLayer::invalidateBuffers()
{
  FullScreenShader::invalidateObject(d->object);
  Layer::invalidateBuffers();
}


}
