#include "tp_maps/layers/PostSSAOLayer.h"
#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct PostSSAOLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::PostSSAOLayer::Private");
  TP_NONCOPYABLE(Private);

  PostSSAOLayer* q;

  //################################################################################################
  Private(PostSSAOLayer* q_):
    q(q_)
  {

  }
};

//##################################################################################################
PostSSAOLayer::PostSSAOLayer(Map* map, RenderPass customRenderPass):
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
PostSSAOLayer::~PostSSAOLayer()
{
  delete d;
}

//##################################################################################################
void PostSSAOLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass())
    return;

  auto shader = map()->getShader<PostSSAOShader>();
  if(shader->error())
    return;

  shader->use();
  shader->setReflectionTextures(map()->reflectionTexture(), map()->reflectionDepth());
  shader->setProjectionMatrix(map()->controller()->matrices(coordinateSystem()).p);
  shader->draw();
}

}
