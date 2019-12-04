#include "tp_maps/layers/PointsLayer.h"

#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/Texture.h"
#include "tp_maps/picking_results/PointsPickingResult.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{

//##################################################################################################
struct PointsLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::PointsLayer::Private");
  TP_NONCOPYABLE(Private);

  PointsLayer* q;

  SpriteTexture* spriteTexture;

  std::vector<PointSpriteShader::PointSprite> points;

  PointSpriteShader::VertexBuffer* vertexBuffer{nullptr};

  GLuint textureID{0};

  bool bindBeforeRender{true};
  bool updateVertexBuffer{true};


  //################################################################################################
  Private(PointsLayer* q_, SpriteTexture* spriteTexture_):
    q(q_),
    spriteTexture(spriteTexture_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(textureID)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(textureID);
    }

    delete spriteTexture;
    deleteVertexBuffers();
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    delete vertexBuffer;
    vertexBuffer=nullptr;
  }
};

//##################################################################################################
PointsLayer::PointsLayer(SpriteTexture *spriteTexture):
  d(new Private(this, spriteTexture))
{
  spriteTexture->texture()->setImageChangedCallback([this]()
  {
    d->bindBeforeRender = true;
    update();
  });

  spriteTexture->setCoordsChangedCallback([this]()
  {
    d->updateVertexBuffer = true;
    update();
  });
}

//##################################################################################################
PointsLayer::~PointsLayer()
{
  delete d;
}

//##################################################################################################
void PointsLayer::clearPoints()
{
  d->points.clear();
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void PointsLayer::setPoints(const std::vector<PointSpriteShader::PointSprite>& points)
{
  d->points = points;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
const std::vector<PointSpriteShader::PointSprite>& PointsLayer::points() const
{
  return d->points;
}

//##################################################################################################
void PointsLayer::render(RenderInfo& renderInfo)
{
  if(!d->spriteTexture->texture()->imageReady())
    return;

  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<PointSpriteShader>();
  if(shader->error())
    return;

  if(d->bindBeforeRender)
  {
    map()->deleteTexture(d->textureID);
    d->textureID = d->spriteTexture->texture()->bindTexture();
    d->bindBeforeRender=false;
  }

  if(!d->textureID)
    return;

  if(d->updateVertexBuffer)
  {
    if(d->vertexBuffer)
    {
      shader->deleteVertexBuffer(d->vertexBuffer);
      d->vertexBuffer = nullptr;
    }

    d->vertexBuffer = shader->generateVertexBuffer(map(), d->points, d->spriteTexture->coords());
    d->updateVertexBuffer=false;
  }

  shader->use(renderInfo.pass==RenderPass::Picking?ShaderType::Picking:ShaderType::Render);
  shader->setMatrix(map()->controller()->matrix(coordinateSystem()));
  shader->setScreenSize(map()->screenSize());
  shader->setTexture(d->textureID);

  map()->controller()->enableScissor(coordinateSystem());
  if(renderInfo.pass==RenderPass::Picking)
  {
    PickingDetails pickingDetails;

    uint32_t pointCount = d->vertexBuffer->indexCount/4;

    auto pickingID = renderInfo.pickingID(PickingDetails(0, [&](const PickingResult& r) -> PickingResult*
    {
      if(r.details.index<d->points.size() && r.renderInfo.map)
      {
        return new PointsPickingResult(r.pickingType, r.details, r.renderInfo, this, r.details.index, d->points.at(r.details.index));
      }
      return nullptr;
    }, pointCount));

    shader->drawPointSpritesPicking(d->vertexBuffer, pickingID);
  }
  else
  {
    shader->drawPointSprites(d->vertexBuffer);
  }
  map()->controller()->disableScissor();
}

//##################################################################################################
void PointsLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  d->textureID = 0;
  d->bindBeforeRender = true;
}

}
