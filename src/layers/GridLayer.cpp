#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/shaders/FontShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/Controller.h"
#include "tp_maps/PreparedString.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{
namespace
{
//##################################################################################################
struct LinesDetails_lt
{
  LineShader::VertexBuffer* vertexBuffer{nullptr};
  glm::vec3 color{0.0f, 0.0f, 0.0f};
  glm::vec3 gridNormal{0.0f, 0.0f, 1.0f};
  float alpha{1.0f};
};
}

//##################################################################################################
struct GridLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::GridLayer::Private");
  TP_NONCOPYABLE(Private);

  GridLayer* q;

  FontRenderer* font{nullptr};

  //Processed geometry ready for rendering
  std::vector<LinesDetails_lt> processedGeometry;

  bool updateVertexBuffer{true};

  float scale; // default: 1 = 1 meter
  const float halfLength = 1.0f; // Half length of the grid.
  float spacing = 0.1f; // Distance between graduation marks. Default: every 0.1 meter
  float heightOffset = 0.001f; // Offset to elevate the grid.
  glm::vec2 horizontalTranslationOffset{0.0f}; // Offset the grid centre on the horizontal plane.
  float horizontalRotationOffset = 0.0f; // Angle (in radian) to rotate the grid on the horizontal plane.
  const glm::vec3 gridColor{0.05f, 0.05f, 0.9f}; // blue
  const float lineThickness = 3.0f; // Note: doesn't work for all OpenGL versions.

  //################################################################################################
  Private(GridLayer* q_,
          float scale_,
          const glm::vec3& gridColor_):
    q(q_),
    scale(scale_),
    gridColor(gridColor_)
  {
  }

  //################################################################################################
  ~Private()
  {
    deleteVertexBuffers();
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    for(const auto& details : processedGeometry)
      delete details.vertexBuffer;

    processedGeometry.clear();
  }

  //################################################################################################
  void calculateGrid(const glm::mat4& matrix_, LinesDetails_lt& details)
  {
    auto matrix = glm::inverse(matrix_);

    float perpendicular{0.0f};
    {
      auto n = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      auto f = matrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
      auto v = glm::vec3((n/n.w)-(f/f.w));

      perpendicular = glm::dot(details.gridNormal, glm::normalize(v));
      perpendicular = tpMax(std::fabs(perpendicular)*2.0f-1.0f, 0.0f);
    }
    details.alpha = perpendicular;
    details.alpha = 1.0f;
  }

  //################################################################################################
  void renderLines(const glm::mat4& matrix)
  {
    auto shader = q->map()->getShader<LineShader>();
    if(shader->error())
      return;

    if(updateVertexBuffer)
    {
      deleteVertexBuffers();
      updateVertexBuffer=false;

      // Number of graduation marks per semi axis.
      size_t graduationCount = halfLength / spacing;
      float cos = 0.0f;
      float sin = 0.0f;
      if (horizontalRotationOffset != 0.0f)
      {
        cos = glm::cos(horizontalRotationOffset);
        sin = glm::sin(horizontalRotationOffset);
      }

      auto drawGraduationsOnAxis = [&](int axisIdx, int directionIdx, const glm::vec3& offset)
      {
        glm::vec3 axisOffset{0,0,0};
        axisOffset[int(axisIdx)] = spacing * scale;

        glm::vec3 directionOffset{0,0,0};
        directionOffset[int(directionIdx)] = graduationCount * axisOffset[int(axisIdx)];
        // Rotate the direction horizontally.
        if (horizontalRotationOffset != 0.0f)
        {
          float prevDirectionOffsetX = directionOffset.x;
          directionOffset.x = cos * directionOffset.x - sin * directionOffset.y;
          directionOffset.y = sin * prevDirectionOffsetX + cos * directionOffset.y;
        }

        std::vector<glm::vec3> vertices;
        // Draw graduation lines on the current axis.
        glm::vec3 gridOrigin = offset;
        for (size_t graduationIdx = 0; graduationIdx < graduationCount; ++graduationIdx)
        {
          auto addLine = [&](float lineIdx)
          {
            auto graduationOrigin = gridOrigin + lineIdx * axisOffset;
            vertices.emplace_back(graduationOrigin - directionOffset);
            vertices.emplace_back(graduationOrigin + directionOffset);
          };
          // Positive side of the axis.
          addLine(float(graduationIdx));
          if (graduationIdx == 0)
            continue;
          // Negative side of the axis.
          addLine(-float(graduationIdx));
        }

        LinesDetails_lt details;
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), vertices);
        details.color = gridColor;
        processedGeometry.push_back(details);
      };

      // Grid on horizontal plane (i.e. xOy). Following Blender coordinate system.
      // Draw graduation lines on x-axis, parallel to y-axis.
      drawGraduationsOnAxis(0, 1, glm::vec3(horizontalTranslationOffset, heightOffset));
      // Draw graduation lines on y-axis, parallel to x-axis.
      drawGraduationsOnAxis(1, 0, glm::vec3(horizontalTranslationOffset, heightOffset));
    }

    shader->use();
    shader->setMatrix(matrix);
    shader->setLineWidth(lineThickness);

    q->map()->controller()->enableScissor(q->coordinateSystem());
    for(const LinesDetails_lt& line : processedGeometry)
    {
      shader->setColor({line.color, line.alpha});
      shader->drawLines(GL_LINES, line.vertexBuffer);
    }
    q->map()->controller()->disableScissor();
  }

  //################################################################################################
  void renderText(const glm::mat4& matrix)
  {
    if(!font)
      return;

    auto shader = q->map()->getShader<FontShader>();
    if(shader->error())
      return;

    shader->use();
    shader->setMatrix(matrix);
  }
};

//##################################################################################################
GridLayer::GridLayer(float scale, const glm::vec3& gridColor):
  d(new Private(this, scale, gridColor))
{
}

//##################################################################################################
GridLayer::~GridLayer()
{
  delete d;
}

//##################################################################################################
void GridLayer::setSpacing(float spacing)
{
  d->spacing = spacing;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
float GridLayer::getSpacing() const
{
  return d->spacing;
}

//##################################################################################################
void GridLayer::setHeightOffset(float heightOffset)
{
  d->heightOffset = heightOffset;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GridLayer::setHorizontalTranslationOffset(const glm::vec2& horizontalTranslationOffset)
{
  d->horizontalTranslationOffset = horizontalTranslationOffset;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GridLayer::setHorizontalRotationOffset(float horizontalRotationOffset)
{
  d->horizontalRotationOffset = horizontalRotationOffset;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GridLayer::setFont(FontRenderer* font)
{
  d->font = font;
}

//##################################################################################################
void GridLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != RenderPass::Normal)
    return;

  glm::mat4 matrix = map()->controller()->matrix(coordinateSystem());

  for(auto& lines : d->processedGeometry)
    d->calculateGrid(matrix, lines);

  d->renderLines(matrix);
  //d->renderText(matrix);
}

//##################################################################################################
void GridLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  Layer::invalidateBuffers();
}

}
