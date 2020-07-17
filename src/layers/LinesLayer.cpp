#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/picking_results/LinesPickingResult.h"

#include "tp_utils/DebugUtils.h"

#include "glm/glm.hpp"

namespace tp_maps
{
namespace
{
struct LinesDetails_lt
{
  LineShader::VertexBuffer* vertexBuffer;
  glm::vec4 color;
  GLenum mode;
};
}

//##################################################################################################
struct LinesLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::LinesLayer::Private");
  TP_NONCOPYABLE(Private);

  LinesLayer* q;
  std::vector<Lines> lines;
  glm::mat4 objectMatrix{1.0f};

  //Processed geometry ready for rendering
  bool updateVertexBuffer{true};
  std::vector<LinesDetails_lt> processedGeometry;
  float lineWidth{1.0f};

  //################################################################################################
  Private(LinesLayer* q_):
    q(q_)
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
};

//##################################################################################################
LinesLayer::LinesLayer():
  d(new Private(this))
{

}

//##################################################################################################
LinesLayer::~LinesLayer()
{
  delete d;
}

//##################################################################################################
const std::vector<Lines>& LinesLayer::lines()const
{
  return d->lines;
}

//##################################################################################################
void LinesLayer::setLines(const std::vector<Lines>& lines)
{
  d->lines = lines;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void LinesLayer::setLinesFromGeometryNormals(const std::vector<Geometry3D>& geometry, float scale)
{
  std::vector<tp_maps::Lines> lines;
  lines.resize(3);
  auto& r = lines.at(0);
  auto& g = lines.at(1);
  auto& b = lines.at(2);

  r.color = {1.0f, 0.0f, 0.0f, 1.0f};
  g.color = {0.0f, 1.0f, 0.0f, 1.0f};
  b.color = {0.0f, 0.0f, 1.0f, 1.0f};

  r.mode = GL_LINES;
  g.mode = GL_LINES;
  b.mode = GL_LINES;

  for(const auto& m : geometry)
  {
    for(const auto& v : m.geometry.verts)
    {
      r.lines.push_back(v.vert);
      g.lines.push_back(v.vert);
      b.lines.push_back(v.vert);

      r.lines.push_back(v.vert + (v.tangent  * scale));
      g.lines.push_back(v.vert + (v.bitangent* scale));
      b.lines.push_back(v.vert + (v.normal   * scale));
    }
  }

  setLines(lines);
}

//##################################################################################################
const glm::mat4& LinesLayer::objectMatrix()const
{
  return d->objectMatrix;
}

//##################################################################################################
void LinesLayer::setObjectMatrix(const glm::mat4& objectMatrix)
{
  d->objectMatrix = objectMatrix;
  update();
}

//##################################################################################################
float LinesLayer::lineWidth()const
{
  return d->lineWidth;
}

//##################################################################################################
void LinesLayer::setLineWidth(float lineWidth)
{
  d->lineWidth = lineWidth;
  update();
}

//##################################################################################################
glm::mat4 LinesLayer::calculateMatrix() const
{
  return map()->controller()->matrix(coordinateSystem()) * d->objectMatrix;
}

//##################################################################################################
void LinesLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<LineShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const Lines& shape : d->lines)
    {
      LinesDetails_lt& details = d->processedGeometry.emplace_back();
      details.vertexBuffer = shader->generateVertexBuffer(map(), shape.lines);
      details.color = shape.color;
      details.mode = shape.mode;
    }
  }

  shader->use(renderInfo.pass==RenderPass::Picking?ShaderType::Picking:ShaderType::Render);
  shader->setMatrix(calculateMatrix());
  shader->setLineWidth(d->lineWidth);

  if(renderInfo.pass==RenderPass::Picking)
  {
    size_t i=0;
    for(const LinesDetails_lt& line : d->processedGeometry)
    {
      auto pickingID = renderInfo.pickingIDMat(PickingDetails(0, [&, i](const PickingResult& r) -> PickingResult*
      {
        return new LinesPickingResult(r.pickingType, r.details, r.renderInfo, this, i);
      }));

      shader->setColor(pickingID);
      shader->drawLines(line.mode, line.vertexBuffer);
      i++;
    }
  }
  else
  {
    for(const LinesDetails_lt& line : d->processedGeometry)
    {
      shader->setColor(line.color);
      shader->drawLines(line.mode, line.vertexBuffer);
    }
  }
}

//##################################################################################################
void LinesLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
