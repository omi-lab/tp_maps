#include "tp_maps/layers/HandleLayer.h"
#include "tp_maps/shaders/PointSpriteShader.h"
#include "tp_maps/picking_results/HandlePickingResult.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/Texture.h"

#include "tp_math_utils/Plane.h"

#include "glm/gtx/norm.hpp" // IWYU pragma: keep

#include <vector>

namespace tp_maps
{

namespace
{
//##################################################################################################
glm::vec2 closestPointOnLine(const glm::vec2& a, const glm::vec2& b, const glm::vec2& p)
{
  glm::vec2 ap = p-a;
  glm::vec2 ab = b-a;

  float magAB2 = ab.x*ab.x + ab.y*ab.y;
  float abDotAP = ab.x*ap.x + ab.y*ap.y;
  float t = abDotAP / magAB2;

  return (t<0.0f)?a:((t>1.0f)?b:glm::vec2(a.x + ab.x*t, a.y + ab.y*t));
}
}

//##################################################################################################
struct HandleLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::HandleLayer::Private");
  TP_NONCOPYABLE(Private);

  HandleLayer* q;

  SpriteTexture* spriteTexture;

  tp_math_utils::Plane plane{{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

  float zOffset{0.0f};

  //The raw data passed to this class
  std::vector<HandleDetails*> handles;

  std::function<bool(HandleDetails*, const glm::ivec2&, const glm::mat4&, glm::vec3&)> calculateHandlePositionCallback;

  //Processed geometry ready for rendering
  bool updateVertexBuffer{true};
  PointSpriteShader::VertexBuffer* vertexBuffer{nullptr};

  //Bound texture details
  bool bindBeforeRender{true};
  GLuint textureID{0};

  int currentHandle{-1};
  glm::ivec2 clickOffset{0,0};

  bool doubleClickToAdd{true};
  bool doubleClickToRemove{true};

  //################################################################################################
  Private(HandleLayer* q_, SpriteTexture* spriteTexture_):
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
HandleDetails::HandleDetails(HandleLayer* layer,
                             const glm::vec3& position_,
                             const glm::vec4& color_,
                             size_t sprite_,
                             float radius_,
                             int index):
  m_layer(layer),
  position(position_),
  color(color_),
  sprite(sprite_),
  radius(radius_),
  opaque(nullptr)
{
  if(index<=m_layer->d->currentHandle)
    m_layer->d->currentHandle++;

  m_layer->d->handles.insert(m_layer->d->handles.begin()+index, this);
  m_layer->d->updateVertexBuffer=true;
  m_layer->update();
}

//##################################################################################################
HandleDetails::~HandleDetails()
{
  if(m_layer)
  {
    size_t index = tpIndexOf(m_layer->d->handles, this);

    if(int(index) < m_layer->d->currentHandle)
      m_layer->d->currentHandle--;

    else if(int(index) == m_layer->d->currentHandle)
      m_layer->d->currentHandle=-1;

    tpRemoveAt(m_layer->d->handles, index);

    m_layer->d->updateVertexBuffer=true;
    m_layer->update();
  }
}

//##################################################################################################
HandleLayer* HandleDetails::layer() const
{
  return m_layer;
}

//##################################################################################################
HandleLayer::HandleLayer(SpriteTexture *spriteTexture):
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
HandleLayer::~HandleLayer()
{
  for(HandleDetails* handle : d->handles)
  {
    handle->m_layer = nullptr;
    delete handle;
  }

  delete d;
}

//##################################################################################################
void HandleLayer::setPlane(const tp_math_utils::Plane& plane)
{
  d->plane = plane;
}

//##################################################################################################
const tp_math_utils::Plane& HandleLayer::plane() const
{
  return d->plane;
}

//##################################################################################################
float HandleLayer::zOffset() const
{
  return d->zOffset;
}

//##################################################################################################
void HandleLayer::setZOffset(float zOffset)
{
  d->zOffset = zOffset;
}

//##################################################################################################
void HandleLayer::setAddRemove(bool doubleClickToAdd, bool doubleClickToRemove)
{
  d->doubleClickToAdd = doubleClickToAdd;
  d->doubleClickToRemove = doubleClickToRemove;
}

//##################################################################################################
const std::vector<HandleDetails*>& HandleLayer::handles() const
{
  return d->handles;
}

//##################################################################################################
void HandleLayer::clearHandles()
{
  while(!d->handles.empty())
    delete d->handles.at(0);
}

//##################################################################################################
void HandleLayer::updateHandles()
{
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void HandleLayer::setCalculateHandlePositionCallback(const std::function<bool(HandleDetails*, const glm::ivec2&, const glm::mat4&, glm::vec3&)>& calculateHandlePositionCallback)
{
  d->calculateHandlePositionCallback = calculateHandlePositionCallback;
}

//##################################################################################################
void HandleLayer::render(RenderInfo& renderInfo)
{
  if(!d->spriteTexture->texture()->imageReady())
    return;

  if(renderInfo.pass != defaultRenderPass().type &&
     renderInfo.pass != RenderPass::Picking)
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

    std::vector<PointSpriteShader::PointSprite> pointSprites;
    pointSprites.reserve(d->handles.size());
    {
      HandleDetails** h = d->handles.data();
      HandleDetails** hMax = h + d->handles.size();
      for(; h<hMax; h++)
      {
        HandleDetails* hh = *h;
        PointSpriteShader::PointSprite ps;
        ps.color       = hh->color;
        ps.position    = hh->position;
        ps.spriteIndex = hh->sprite;
        ps.radius      = hh->radius;
        ps.offset.x    = hh->offset.x;
        ps.offset.y    = hh->offset.y;
        ps.offset.z    = d->zOffset;
        pointSprites.push_back(ps);
      }
    }

    d->vertexBuffer = shader->generateVertexBuffer(map(), pointSprites, d->spriteTexture->coords());
    d->updateVertexBuffer=false;
  }

  shader->use(renderInfo.shaderType());

  glm::mat4 m = map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();

  shader->setMatrix(m);
  shader->setScreenSize(map()->screenSize());
  shader->setTexture(d->textureID);

  map()->controller()->enableScissor(coordinateSystem());
  if(renderInfo.pass==RenderPass::Picking)
  {
    auto pickingID = renderInfo.pickingID(PickingDetails(0, [this](const PickingResult& r) -> PickingResult*
    {
      if(r.details.index<d->handles.size() && r.renderInfo.map)
      {
        HandleDetails* handle = d->handles.at(r.details.index);
        return new HandlePickingResult(r.pickingType, r.details, r.renderInfo, handle);
      }
      return nullptr;
    }, uint32_t(d->handles.size())));
    shader->drawPointSpritesPicking(d->vertexBuffer, pickingID);
  }
  else
  {
    shader->drawPointSprites(d->vertexBuffer);
  }
  map()->controller()->disableScissor();
}

//##################################################################################################
void HandleLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  d->textureID = 0;
  d->bindBeforeRender = true;
  Layer::invalidateBuffers();
}

//##################################################################################################
bool HandleLayer::mouseEvent(const MouseEvent& event)
{
  glm::mat4 m = map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();

  switch(event.type)
  {
  case MouseEventType::Press: //--------------------------------------------------------------------
  {
    if(event.button != Button::LeftButton)
      return false;

    float width   = float(map()->width());
    float height  = float(map()->height());
    float xOffset = width  / 2.0f;
    float yOffset = height / 2.0f;

    PickingResult* result = map()->performPicking(gizmoLayerSID(), event.pos);
    TP_CLEANUP([&]{delete result;});

    auto pickingResult = dynamic_cast<HandlePickingResult*>(result);
    if(pickingResult && pickingResult->layer == this)
    {
      for(size_t i=0; i<d->handles.size(); i++)
      {
        const auto& handle = d->handles.at(i);
        if(handle == pickingResult->handle)
        {
          d->currentHandle = int(i);

          const glm::vec3& position = handle->position;

          glm::vec2 screenCoord = tpProj(m, position);
          screenCoord.x = screenCoord.x * xOffset + xOffset;
          screenCoord.y = height - (screenCoord.y * yOffset + yOffset);
          screenCoord += handle->offset*handle->radius;

          d->clickOffset = event.pos - glm::ivec2(screenCoord);

          dragStart(pickingResult->handle);
          return true;
        }
      }
    }

#if 0 //If not picking ....

    size_t hMax = d->handles.size();
    for(size_t h=0; h<hMax; h++)
    {
      const glm::vec3& position = d->handles.at(h)->position;

      glm::vec4 screenCoord = m * glm::vec4(position, 1.0f);
      screenCoord.x = screenCoord.x * xOffset + xOffset;
      screenCoord.y = height - (screenCoord.y * yOffset + yOffset);

      int xDiff{int(std::abs(screenCoord.x - float(event.pos.x)))};
      int yDiff{int(std::abs(screenCoord.y - float(event.pos.y)))};

      int manhattanLength = xDiff + yDiff;

      if(manhattanLength < 16)
      {
        d->currentHandle = int(h);
        return true;
      }
    }
#endif

    break;
  }

  case MouseEventType::Move: //---------------------------------------------------------------------
  {
    if(d->currentHandle>=0 && d->currentHandle<int(d->handles.size()))
    {
      glm::ivec2 offsetPos = event.pos - d->clickOffset;
      glm::vec3 newPosition;
      if(d->calculateHandlePositionCallback)
      {
        if(d->calculateHandlePositionCallback(d->handles[size_t(d->currentHandle)], offsetPos, m, newPosition))
        {
          moveHandle(d->handles[size_t(d->currentHandle)], newPosition);
          return true;
        }
      }
      else if(map()->unProject(offsetPos, newPosition, d->plane, m))
      {
        moveHandle(d->handles[size_t(d->currentHandle)], newPosition);
        return true;
      }
    }
    break;
  }

  case MouseEventType::Release: //------------------------------------------------------------------
  {
    if(d->currentHandle != -1)
    {
      d->currentHandle = -1;
      return true;
    }
    break;
  }

  case MouseEventType::Wheel: //--------------------------------------------------------------------
  {
    break;
  }

  case MouseEventType::DoubleClick: //--------------------------------------------------------------
  {
    if(event.button == Button::LeftButton)
    {
      if(!d->doubleClickToAdd)
        break;

      if(d->handles.size()<2)
        break;

      glm::vec3 p3;
      if(!map()->unProject(event.pos, p3, d->plane, m))
        break;

      glm::vec2 p(p3.x, p3.y);

      float dist=1000000.0f;
      int index=-1;
      glm::vec2 closest;
      auto calc = [&](const glm::vec3& a3, const glm::vec3& b3, int j)
      {
        glm::vec2 a(a3.x, a3.y);
        glm::vec2 b(b3.x, b3.y);

        glm::vec2 c = closestPointOnLine(a, b, p);
        glm::vec2 v = p-c;
        float distSq = v.x*v.x + v.y*v.y;

        if(distSq<dist)
        {
          glm::vec2 cScreen;
          map()->project(glm::vec3(c, 0.0f), cScreen, m);
          constexpr float maxDistance2 = (6.0f*6.0f);
          if(glm::distance2(glm::vec2(event.pos), cScreen) < maxDistance2)
          {
            dist = distSq;
            index = j;
            closest = c;
          }
        }
      };

      for(int i=1; i<int(d->handles.size()); i++)
        calc(d->handles.at(size_t(i-1))->position, d->handles.at(size_t(i))->position, i);

      calc(d->handles.at(d->handles.size()-1)->position, d->handles.at(0)->position, int(d->handles.size()));

      if(index>=0)
      {
        HandleDetails* style = d->handles.at(0);
        HandleDetails* handle = new HandleDetails(this,
                                                  glm::vec3(p.x, p.y, 0.0f),
                                                  style->color,
                                                  style->sprite,
                                                  style->radius,
                                                  index);
        moveHandle(handle, handle->position);

        return true;
      }
    }

    else if(event.button == Button::RightButton)
    {
      if(!d->doubleClickToRemove)
        break;

      PickingResult* result = map()->performPicking(gizmoLayerSID(), event.pos);
      TP_CLEANUP([&]{delete result;});

      auto pickingResult = dynamic_cast<HandlePickingResult*>(result);

      if(pickingResult && pickingResult->layer == this && d->handles.size()>3)
      {
        handleDeleted(pickingResult->handle);
        delete pickingResult->handle;
      }
    }

    break;
  }

  case MouseEventType::Click: //--------------------------------------------------------------------
  {
    break;
  }

  case MouseEventType::DragStart: //----------------------------------------------------------------
  {
    break;
  }

  }

  return false;
}

//##################################################################################################
void HandleLayer::moveHandle(HandleDetails* handle, const glm::vec3& newPosition)
{
  handle->position = newPosition;

  handleMoved(handle);

  d->updateVertexBuffer=true;
  update();
}

}
