#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/layers/HandleLayer.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/textures/DefaultSpritesTexture.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/Controller.h"

#include <vector>

namespace tp_maps
{
namespace
{
//##################################################################################################
struct LinesDetails_lt
{
  LineShader::VertexBuffer* vertexBuffer{nullptr};
  glm::vec4 color{0.0f, 0.0f, 0.0f, 1.0f};
};

//##################################################################################################
enum class Orientation_lt
{
  Horizontal,
  Vertical
};

//##################################################################################################
struct DragLineDetails_lt
{
  Orientation_lt orientation{Orientation_lt::Horizontal};
  glm::vec3 initialPosition{0.0f, 0.0f, 0.0f};
  HandleDetails* handleA{nullptr};
  HandleDetails* handleB{nullptr};

  //################################################################################################
  bool owns(HandleDetails* h)
  {
    return handleA==h or handleB==h;
  }

  //################################################################################################
  float value()
  {
    if(orientation == Orientation_lt::Horizontal)
      return handleA->position.y;
    else
      return handleA->position.x;
  }
};

//##################################################################################################
struct DragLinePair_lt
{
  DragLineDetails_lt a;
  DragLineDetails_lt b;

  //################################################################################################
  bool owns(HandleDetails* h)
  {
    return a.owns(h) or b.owns(h);
  }

  //################################################################################################
  float value()
  {
    return std::fabs(a.value());
  }
};
}

//##################################################################################################
struct GridLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::GridLayer::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  HandleLayer* handleLayer{nullptr};

  //Processed geometry ready for rendering
  std::vector<LinesDetails_lt> processedGeometry;

  bool updateVertexBuffer{true};

  GridMode mode{GridMode::User};
  GridAxis axis{GridAxis::ZPlane};
  GridHandles handles{GridHandles::None};

  bool initDragLines{true};

  std::vector<DragLineDetails_lt> staticDragLines;
  std::vector<DragLinePair_lt> freeDragLines;

  float scale{1.0f};
  const float halfLength = 1.0f;
  float spacing = 0.1f;
  float heightOffset = 0.001f;
  glm::vec2 horizontalTranslationOffset{0.0f};
  glm::vec2 horizontalOrientation{0.0f, 1.0f};
  const glm::vec3 gridColor{0.05f, 0.05f, 0.9f};
  const float lineThickness = 1.0f;

  std::function<tp_maps::SpriteTexture*(tp_maps::Map*)> makeTexture;

  //################################################################################################
  Private(Q* q_, const std::function<tp_maps::SpriteTexture*(tp_maps::Map*)>& makeTexture_):
    q(q_),
    makeTexture(makeTexture_)
  {
    if(!makeTexture) makeTexture = [](tp_maps::Map* map)
    {
      auto t = new tp_maps::SpriteTexture();
      t->setTexture(new tp_maps::DefaultSpritesTexture(map));
      return t;
    };
  }

  //################################################################################################
  ~Private()
  {
    deleteVertexBuffers();
  }

  //################################################################################################
  void updateCoordinateSystem()
  {
    tp_utils::StringID coordinateSystem;

    switch(axis)
    {
      case GridAxis::Frame: [[fallthrough]];
      case GridAxis::Screen:
      coordinateSystem = "Mask";//tp_maps::screenSID();
      break;

      case GridAxis::XPlane: [[fallthrough]];
      case GridAxis::YPlane: [[fallthrough]];
      case GridAxis::ZPlane:
      coordinateSystem = tp_maps::defaultSID();
      break;
    }

    q->setCoordinateSystem(coordinateSystem);

    if(handleLayer)
      handleLayer->setCoordinateSystem(coordinateSystem);
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    for(const auto& details : processedGeometry)
      delete details.vertexBuffer;

    processedGeometry.clear();
  }

  //################################################################################################
  void generateLines_Fixed(LineShader* shader)
  {
    auto drawGraduationsOnAxis = [&](const glm::vec3& axisToAddGraduations,  const glm::vec3& lineDirection, const glm::vec3& offset, float spaceBetweenGraduations, size_t graduationCount)
    {
      glm::vec3 axisOffset = spaceBetweenGraduations * axisToAddGraduations;
      glm::vec3 directionOffset = graduationCount * spaceBetweenGraduations * lineDirection;

      std::vector<glm::vec3> centralLinesVertices;
      std::vector<glm::vec3> linesVertices;
      glm::vec3 gridOrigin = offset;

      auto addLine = [&](int lineIdx, std::vector<glm::vec3>& vertices)
      {
        auto graduationOrigin = gridOrigin + float(lineIdx) * axisOffset;
        vertices.emplace_back(graduationOrigin - directionOffset);
        vertices.emplace_back(graduationOrigin + directionOffset);
      };

      // Draw graduation lines on the current axis.
      for (size_t graduationIdx = 0; graduationIdx < graduationCount; ++graduationIdx)
      {
        if (graduationIdx == 0)
        {
          addLine(int(graduationIdx), centralLinesVertices);
        }
        else
        {
          // Positive side of the axis.
          addLine(int(graduationIdx), linesVertices);
          // Negative side of the axis.
          addLine(-int(graduationIdx), linesVertices);
        }
      }

      // Central lines (in red)
      {
        LinesDetails_lt& details = processedGeometry.emplace_back();
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), centralLinesVertices);
        details.color = {1.0f, 0.0f, 0.0f, 1.0f};
      }

      // Side lines
      {
        LinesDetails_lt& details = processedGeometry.emplace_back();
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), linesVertices);
        details.color = {gridColor, 1.0f};
      }
    };

    glm::vec3 forwardAxis{0.0f, 1.0f, 0.0f}; // y-axix
    glm::vec3 rightAxis{1.0f, 0.0f, 0.0f}; // x-axis

    //    if (gridAs2DOverlay)
    //    {
    //      float ratio = 1.0f;
    //      if (q->map()->height() != 0)
    //        ratio = float(q->map()->width()) / float(q->map()->height());
    //      float spacing2D = spacing * 4.0f;
    //      float spaceBetweenGraduations = spacing2D;
    //      size_t graduationCount = size_t(halfLength / spacing2D + 1.0f);
    //      // Draw graduation lines on x-axis, parallel to y-axis.
    //      drawGraduationsOnAxis(rightAxis, forwardAxis, glm::vec3(0.0f, 0.0f, 0.0f),
    //                            spaceBetweenGraduations / ratio, size_t(graduationCount * ratio));
    //      // Draw graduation lines on y-axis, parallel to x-axis.
    //      drawGraduationsOnAxis(forwardAxis, rightAxis, glm::vec3(0.0f, 0.0f, 0.0f),
    //                            spaceBetweenGraduations, graduationCount);
    //    }
    //    else
    {
      if (horizontalOrientation != glm::vec2(0.0f))
      {
        forwardAxis = glm::normalize(glm::vec3(horizontalOrientation, 0.0f));
        rightAxis = glm::normalize(glm::vec3(horizontalOrientation.y, -horizontalOrientation.x, 0.0f));
      }
      float spaceBetweenGraduations = spacing * scale;
      size_t graduationCount = size_t(halfLength / spacing + 1.0f);

      // Grid on horizontal plane (i.e. xOy). Following Blender coordinate system.
      // Draw graduation lines on x-axis, parallel to y-axis.
      drawGraduationsOnAxis(rightAxis, forwardAxis, glm::vec3(horizontalTranslationOffset, heightOffset), spaceBetweenGraduations, graduationCount);
      // Draw graduation lines on y-axis, parallel to x-axis.
      drawGraduationsOnAxis(forwardAxis, rightAxis, glm::vec3(horizontalTranslationOffset, heightOffset), spaceBetweenGraduations, graduationCount);
    }
  }
  //################################################################################################
  void generateLines_User(LineShader* shader)
  {
    std::vector<glm::vec3> verts;
    verts.reserve(freeDragLines.size()*4);
    for(const auto& dragLinePair : freeDragLines)
    {
      auto addLine = [&](const DragLineDetails_lt& dragLine)
      {
        verts.push_back(dragLine.handleA->position);
        verts.push_back(dragLine.handleB->position);
      };

      addLine(dragLinePair.a);
      addLine(dragLinePair.b);
    }

    {
      LinesDetails_lt& details = processedGeometry.emplace_back();
      details.vertexBuffer = shader->generateVertexBuffer(q->map(), verts);
      details.color = {1.0f, 0.0f, 0.0f, 1.0f};
    }
  }

  //################################################################################################
  void renderLines(RenderInfo& renderInfo, const glm::mat4& matrix)
  {
    auto shader = q->map()->getShader<LineShader>();
    if(shader->error())
      return;

    if(updateVertexBuffer)
    {
      deleteVertexBuffers();
      updateVertexBuffer=false;

      switch(mode)
      {
        case GridMode::Fixed:
        generateLines_Fixed(shader);
        break;

        case GridMode::User:
        generateLines_User(shader);
        break;
      }
    }

    shader->use(renderInfo.shaderType());
    shader->setMatrix(matrix);

    q->map()->controller()->enableScissor(q->coordinateSystem());
    for(const LinesDetails_lt& line : processedGeometry)
    {
      shader->setColor(line.color);
      shader->drawLines(GL_LINES, line.vertexBuffer);
    }
    q->map()->controller()->disableScissor();
  }

  //################################################################################################
  void createHandleLayer()
  {
    handleLayer = new tp_maps::HandleLayer(makeTexture(q->map()));
    handleLayer->setAddRemove(false, false);
    handleLayer->setCalculateHandlePositionCallback(calculateHandlePosition);
    handleLayer->setDefaultRenderPass(q->defaultRenderPass());
    q->addChildLayer(handleLayer);
    updateCoordinateSystem();
  }

  //################################################################################################
  const std::function<bool(tp_maps::HandleDetails*, const glm::ivec2&, const glm::mat4&, glm::vec3&)> calculateHandlePosition = [&](tp_maps::HandleDetails* h, const glm::ivec2& pos, const glm::mat4& m, glm::vec3& newPosition)
  {
    updateVertexBuffer = true;

    auto update = [&](DragLinePair_lt& newDragLinePair)
    {
      q->map()->unProject(pos, newPosition, handleLayer->plane(), m);
      setDragLinePairPosition(newDragLinePair, newPosition, h);
    };

    auto cloneAndUpdate = [&](DragLineDetails_lt& dragLine)
    {
      DragLinePair_lt& newDragLinePair = freeDragLines.emplace_back();
      newDragLinePair.a = dragLine;
      createDragLine(newDragLinePair.b, newDragLinePair.a.orientation, newDragLinePair.a.initialPosition);

      createDragLine(dragLine, dragLine.orientation, dragLine.initialPosition);
      update(newDragLinePair);
    };

    for(DragLineDetails_lt& dragLine : staticDragLines)
    {
      if(dragLine.owns(h))
      {
        cloneAndUpdate(dragLine);
        return false;
      }
    }

    for(size_t i=0; i<freeDragLines.size(); i++)
    {
      DragLinePair_lt& dragLinePair = freeDragLines.at(i);

      if(dragLinePair.owns(h))
      {
        update(dragLinePair);

        if(dragLinePair.value()>=1.0f)
        {
          delete dragLinePair.a.handleA;
          delete dragLinePair.a.handleB;
          delete dragLinePair.b.handleA;
          delete dragLinePair.b.handleB;
          tpRemoveAt(freeDragLines, i);
        }

        return false;
      }
    }

    return false;
  };

  //################################################################################################
  void createDragLine(DragLineDetails_lt& dragLine, Orientation_lt orientation, const glm::vec3& initialPosition)
  {
    dragLine.orientation = orientation;
    dragLine.initialPosition = initialPosition;

    if(dragLine.orientation == Orientation_lt::Horizontal)
    {
      dragLine.handleA = new HandleDetails(handleLayer, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 2, 10.0f);
      dragLine.handleB = new HandleDetails(handleLayer, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 3, 10.0f);

      dragLine.handleA->offset.x = -1.0f;
      dragLine.handleB->offset.x =  1.0f;
    }
    else
    {
      dragLine.handleA = new HandleDetails(handleLayer, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 0, 10.0f);
      dragLine.handleB = new HandleDetails(handleLayer, {0.0f,  1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, 1, 10.0f);

      dragLine.handleA->offset.y = -1.0f;
      dragLine.handleB->offset.y =  1.0f;
    }

    setDragLinePosition(dragLine, initialPosition);
  }

  //################################################################################################
  void setDragLinePosition(DragLineDetails_lt& dragLine, const glm::vec3& position)
  {
    if(dragLine.orientation == Orientation_lt::Horizontal)
    {
      dragLine.handleA->position = {-1.0f, position.y, 0.0f};
      dragLine.handleB->position = { 1.0f, position.y, 0.0f};
    }
    else
    {
      dragLine.handleA->position = {position.x, -1.0f, 0.0f};
      dragLine.handleB->position = {position.x,  1.0f, 0.0f};
    }

    handleLayer->updateHandles();
  }

  //################################################################################################
  void setDragLinePairPosition(DragLinePair_lt& dragLinePair, const glm::vec3& position, tp_maps::HandleDetails* h)
  {
    if(dragLinePair.a.owns(h))
    {
      setDragLinePosition(dragLinePair.a,  position);
      setDragLinePosition(dragLinePair.b, -position);
    }
    else
    {
      setDragLinePosition(dragLinePair.a, -position);
      setDragLinePosition(dragLinePair.b,  position);
    }
  }

  //################################################################################################
  void updateHandles()
  {
    if(initDragLines)
    {
      initDragLines = false;

      staticDragLines.reserve(4);

      createDragLine(staticDragLines.emplace_back(), Orientation_lt::Horizontal, {0.0f,  1.0f, 0.0f});
      createDragLine(staticDragLines.emplace_back(), Orientation_lt::Horizontal, {0.0f, -1.0f, 0.0f});

      createDragLine(staticDragLines.emplace_back(), Orientation_lt::Vertical, {-1.0f, 0.0f, 0.0f});
      createDragLine(staticDragLines.emplace_back(), Orientation_lt::Vertical, { 1.0f, 0.0f, 0.0f});
    }
  }
};

//##################################################################################################
GridLayer::GridLayer(const std::function<tp_maps::SpriteTexture*(tp_maps::Map*)>& makeTexture):
  d(new Private(this, makeTexture))
{

}

//##################################################################################################
GridLayer::~GridLayer()
{
  delete d;
}

//##################################################################################################
void GridLayer::setMode(GridMode mode)
{
  if(d->mode != mode)
  {
    d->mode = mode;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
GridMode GridLayer::mode() const
{
  return d->mode;
}

//##################################################################################################
void GridLayer::setAxis(GridAxis axis)
{
  if(d->axis != axis)
  {
    d->updateCoordinateSystem();
    d->axis = axis;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
GridAxis GridLayer::axis() const
{
  return d->axis;
}

//##################################################################################################
void GridLayer::setHandles(GridHandles handles)
{
  if(d->handles != handles)
  {
    d->handles = handles;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
GridHandles GridLayer::handles() const
{
  return d->handles;
}

//##################################################################################################
void GridLayer::setSpacing(float spacing)
{
  if(spacing != d->spacing)
  {
    d->spacing = spacing;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
float GridLayer::spacing() const
{
  return d->spacing;
}

//##################################################################################################
void GridLayer::setHeightOffset(float heightOffset)
{
  if (heightOffset != d->heightOffset)
  {
    d->heightOffset = heightOffset;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
float GridLayer::heightOffset() const
{
  return d->heightOffset;
}

//##################################################################################################
void GridLayer::setHorizontalTranslationOffset(const glm::vec2& horizontalTranslationOffset)
{
  d->horizontalTranslationOffset = horizontalTranslationOffset;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GridLayer::setHorizontalOrientation(const glm::vec2& horizontalOrientation)
{
  d->horizontalOrientation = horizontalOrientation;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GridLayer::setDefaultRenderPass(const RenderPass& defaultRenderPass)
{
  Layer::setDefaultRenderPass(defaultRenderPass);

  if(d->handleLayer)
    d->handleLayer->setDefaultRenderPass(defaultRenderPass);
}

//##################################################################################################
void GridLayer::addedToMap()
{
  Layer::addedToMap();
  d->createHandleLayer();
}

//##################################################################################################
void GridLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass == RenderPass::PreRender)
  {
    d->updateHandles();
  }

  Layer::render(renderInfo);

  if(renderInfo.pass != defaultRenderPass())
    return;

  glm::mat4 matrix = map()->controller()->matrix(coordinateSystem());

  d->renderLines(renderInfo, matrix);
}

//##################################################################################################
void GridLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  Layer::invalidateBuffers();
}

}
