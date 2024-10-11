#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/layers/HandleLayer.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/textures/DefaultSpritesTexture.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"

#include <vector>

namespace tp_maps
{
namespace
{
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

  LinesLayer* linesLayer{nullptr};
  HandleLayer* handleLayer{nullptr};

  bool updateVertexBuffer{true};

  GridMode mode{GridMode::User};
  GridAxis axis{GridAxis::ZPlane};
  GridHandles handles{GridHandles::None};

  bool initDragLines{true};

  GridPredefinedLines predefinedLines;

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

  GridColors colors;

  std::function<SpriteTexture*(Map*)> makeTexture;

  //################################################################################################
  Private(Q* q_, const std::function<SpriteTexture*(Map*)>& makeTexture_):
    q(q_),
    makeTexture(makeTexture_)
  {
    if(!makeTexture) makeTexture = [](Map* map)
    {
      auto t = new SpriteTexture();
      t->setTexture(new DefaultSpritesTexture(map));
      return t;
    };
  }

  //################################################################################################
  void updateCoordinateSystem()
  {
    tp_utils::StringID coordinateSystem;

    switch(axis)
    {
      case GridAxis::Frame:
      coordinateSystem = maskSID();
      break;

      case GridAxis::Screen:
      coordinateSystem = screenSID();
      break;

      case GridAxis::XPlane: [[fallthrough]];
      case GridAxis::YPlane: [[fallthrough]];
      case GridAxis::ZPlane:
      coordinateSystem = defaultSID();
      break;
    }

    q->setCoordinateSystem(coordinateSystem);

    if(linesLayer)
      linesLayer->setCoordinateSystem(coordinateSystem);

    if(handleLayer)
      handleLayer->setCoordinateSystem(coordinateSystem);
  }

  //################################################################################################
  void generateLines_Fixed()
  {
    std::vector<Lines> lines;
    TP_CLEANUP([&]{linesLayer->setLines(lines);});

    auto drawGraduationsOnAxis = [&](const glm::vec3& axisToAddGraduations, const glm::vec3& lineDirection)
    {
      glm::vec3 offset{horizontalTranslationOffset, heightOffset};

      size_t intermediates = 10;

      float spaceBetweenGraduations = spacing * scale;
      size_t graduationCount = size_t(halfLength / spacing + 1.0f);

      glm::vec3 axisOffset = (spaceBetweenGraduations / float(intermediates)) * axisToAddGraduations;
      glm::vec3 directionOffset = graduationCount * spaceBetweenGraduations * lineDirection;

      graduationCount *= intermediates;

      lines.resize(3);
      Lines& centralLines = lines.at(0);
      Lines& primaryLines = lines.at(1);
      Lines& intermediateLines = lines.at(2);

      centralLines.mode = GL_LINES;
      centralLines.color = colors.centralLines;

      primaryLines.mode = GL_LINES;
      primaryLines.color = colors.primaryLines;

      intermediateLines.mode = GL_LINES;
      intermediateLines.color = colors.intermediateLines;

      glm::vec3 gridOrigin = offset;

      auto addLine = [&](int lineIdx, std::vector<glm::vec3>& vertices)
      {
        auto graduationOrigin = gridOrigin + float(lineIdx) * axisOffset;
        vertices.emplace_back(graduationOrigin - directionOffset);
        vertices.emplace_back(graduationOrigin + directionOffset);
      };

      // Draw graduation lines on the current axis.
      for (size_t graduationIdx = 0; graduationIdx <= graduationCount; graduationIdx++)
      {
        if(graduationIdx == 0)
        {
          addLine(int(graduationIdx), centralLines.lines);
        }
        else if(graduationIdx%intermediates == 0)
        {
          addLine( int(graduationIdx), primaryLines.lines);
          addLine(-int(graduationIdx), primaryLines.lines);
        }
        else
        {
          addLine( int(graduationIdx), intermediateLines.lines);
          addLine(-int(graduationIdx), intermediateLines.lines);
        }
      }
    };

    glm::vec3 xAxis{1.0f, 0.0f, 0.0f};
    glm::vec3 yAxis{0.0f, 1.0f, 0.0f};
    {
      if (horizontalOrientation != glm::vec2(0.0f))
      {
        yAxis = glm::normalize(glm::vec3(horizontalOrientation, 0.0f));
        xAxis = glm::normalize(glm::vec3(horizontalOrientation.y, -horizontalOrientation.x, 0.0f));
      }

      drawGraduationsOnAxis(xAxis, yAxis);
      drawGraduationsOnAxis(yAxis, xAxis);
    }
  }

  //################################################################################################
  void generateLines_Predefined()
  {
    std::vector<Lines> lines;
    TP_CLEANUP([&]{linesLayer->setLines(lines);});

    lines.resize(1);
    Lines& verts = lines.at(0);

    verts.mode = GL_LINES;
    verts.color = colors.userLines;
    verts.lines.reserve(freeDragLines.size()*4);

    for(const auto& y : predefinedLines.horizontalLines)
    {
      verts.lines.push_back({-1.0f, y, 0.0f});
      verts.lines.push_back({ 1.0f, y, 0.0f});
    }

    for(const auto& x : predefinedLines.verticalLines)
    {
      verts.lines.push_back({x, -1.0f, 0.0f});
      verts.lines.push_back({x,  1.0f, 0.0f});
    }
  }

  //################################################################################################
  void generateLines_User()
  {
    std::vector<Lines> lines;
    TP_CLEANUP([&]{linesLayer->setLines(lines);});

    lines.resize(1);
    Lines& verts = lines.at(0);

    verts.mode = GL_LINES;
    verts.color = colors.userLines;
    verts.lines.reserve(freeDragLines.size()*4);

    for(const auto& dragLinePair : freeDragLines)
    {
      auto addLine = [&](const DragLineDetails_lt& dragLine)
      {
        verts.lines.push_back(dragLine.handleA->position);
        verts.lines.push_back(dragLine.handleB->position);
      };

      addLine(dragLinePair.a);
      addLine(dragLinePair.b);
    }
  }

  //################################################################################################
  void generateLines()
  {
    if(updateVertexBuffer)
    {
      updateVertexBuffer=false;

      switch(mode)
      {
        case GridMode::Fixed:
        generateLines_Fixed();
        break;

        case GridMode::Predefined:
        generateLines_Predefined();
        break;

        case GridMode::User:
        generateLines_User();
        break;
      }
    }
  }

  //################################################################################################
  void createLayers()
  {
    {
      linesLayer = new LinesLayer();
      linesLayer->setDefaultRenderPass(q->defaultRenderPass());
      q->addChildLayer(linesLayer);
    }

    {
      handleLayer = new HandleLayer(makeTexture(q->map()));
      handleLayer->setAddRemove(false, false);
      handleLayer->setCalculateHandlePositionCallback(calculateHandlePosition);
      handleLayer->setDefaultRenderPass(q->defaultRenderPass());
      q->addChildLayer(handleLayer);
    }

    updateCoordinateSystem();
  }

  //################################################################################################
  const std::function<bool(HandleDetails*, const glm::ivec2&, const glm::mat4&, glm::vec3&)> calculateHandlePosition = [&](HandleDetails* h, const glm::ivec2& pos, const glm::mat4& m, glm::vec3& newPosition)
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
    if(mode == GridMode::User and initDragLines)
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
void GridLayer::setPredefinedLines(const GridPredefinedLines& predefinedLines)
{
  d->predefinedLines = predefinedLines;
}

//##################################################################################################
const GridPredefinedLines& GridLayer::predefinedLines() const
{
  return d->predefinedLines;
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

  if(d->linesLayer)
    d->linesLayer->setDefaultRenderPass(defaultRenderPass);

  if(d->handleLayer)
    d->handleLayer->setDefaultRenderPass(defaultRenderPass);
}

//##################################################################################################
void GridLayer::addedToMap()
{
  Layer::addedToMap();
  d->createLayers();
}

//##################################################################################################
void GridLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass == RenderPass::PreRender)
  {
    d->updateHandles();
    d->generateLines();
  }

  Layer::render(renderInfo);
}

}
