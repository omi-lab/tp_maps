#include "tp_maps/layers/PostLayer.h"
#include "tp_maps/shaders/PostShader.h"
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

  //################################################################################################
  Private(PostLayer* q_):
    q(q_)
  {

  }
};

//##################################################################################################
PostLayer::PostLayer(Map* map, RenderPass customRenderPass):
  d(new Private(this))
{
  setDefaultRenderPass(customRenderPass);
  map->setCustomRenderPass(customRenderPass, [](RenderInfo&)
  {
    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);
  },[](RenderInfo&)
  {

  });
}

//##################################################################################################
PostLayer::~PostLayer()
{
  delete d;
}

//##################################################################################################
void PostLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass())
    return;

  auto shader = makeShader();
  if(shader->error())
    return;

  shader->use();
  shader->setReflectionTextures(map()->reflectionTexture(), map()->reflectionDepth());
  shader->setProjectionMatrix(map()->controller()->matrices(coordinateSystem()).p);
  shader->draw();
}

}
