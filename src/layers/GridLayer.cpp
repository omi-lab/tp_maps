#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/shaders/FontShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/Controller.h"
#include "tp_maps/PreparedString.h"

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
  float heightOffset = 0.001f; // Offset to elevate the grid (to make sure it doesn't blend inside the floor).
  glm::vec2 horizontalTranslationOffset{0.0f}; // Offset the grid centre on the horizontal plane.
  glm::vec2 horizontalOrientation{0.0f, 1.0f}; // Direction of the grid on the xOy plane (looking towards y-axis by default).
  const glm::vec3 gridColor{0.05f, 0.05f, 0.9f}; // blue
  const float lineThickness = 3.0f; // Note: doesn't work for all OpenGL versions.
  bool gridAs2DOverlay = false;

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
      //size_t graduationCount = halfLength / spacing;
      //float spaceBetweenGraduations = spacing * scale;

      //auto drawGraduationsOnAxis = [&](const glm::vec3& axisToAddGraduations, const glm::vec3& lineDirection, const glm::vec3& offset)
      auto drawGraduationsOnAxis = [&](const glm::vec3& axisToAddGraduations, const glm::vec3& lineDirection,
          const glm::vec3& offset, float spaceBetweenGraduations, size_t graduationCount)
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

        LinesDetails_lt details;
        // Central lines (in red)
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), centralLinesVertices);
        details.color = glm::vec3(1.0f, 0.0f, 0.0f);
        processedGeometry.push_back(details);

        // Side lines
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), linesVertices);
        details.color = gridColor;
        processedGeometry.push_back(details);
      };

      glm::vec3 forwardAxis{0.0f, 1.0f, 0.0f}; // y-axix
      glm::vec3 rightAxis{1.0f, 0.0f, 0.0f}; // x-axis

      if (gridAs2DOverlay)
      {
        float ratio = 1.0f;
        if (q->map()->height() != 0)
          ratio = float(q->map()->width()) / float(q->map()->height());
        float spacing2D = spacing * 4.0f;
        float spaceBetweenGraduations = spacing2D;
        size_t graduationCount = size_t(halfLength / spacing2D + 1.0f);
        // Draw graduation lines on x-axis, parallel to y-axis.
        drawGraduationsOnAxis(rightAxis, forwardAxis, glm::vec3(0.0f, 0.0f, 0.0f),
                              spaceBetweenGraduations / ratio, size_t(graduationCount * ratio));
        // Draw graduation lines on y-axis, parallel to x-axis.
        drawGraduationsOnAxis(forwardAxis, rightAxis, glm::vec3(0.0f, 0.0f, 0.0f),
                              spaceBetweenGraduations, graduationCount);
      }
      else
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
  if (spacing != d->spacing)
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
void GridLayer::setGridAs2DOverlay(bool gridAs2DOverlay)
{
  if (d->gridAs2DOverlay != gridAs2DOverlay)
  {
    d->gridAs2DOverlay = gridAs2DOverlay;
    //sd->spacing = (d->gridAs2DOverlay)?0.2f:0.1f;
    d->updateVertexBuffer = true;
    update();
  }
}

//##################################################################################################
float GridLayer::gridAs2DOverlay() const
{
  return d->gridAs2DOverlay;
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
void GridLayer::setFont(FontRenderer* font)
{
  d->font = font;
}

//##################################################################################################
void GridLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != RenderPass::Normal)
    return;

  glm::mat4 matrix = glm::mat4(1.0);
  if (!d->gridAs2DOverlay)
    matrix = map()->controller()->matrix(coordinateSystem());

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
