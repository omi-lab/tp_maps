#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/picking_results/LinesPickingResult.h"

#include "tp_math_utils/materials/OpenGLMaterial.h"

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

  Q* q;
  std::vector<Lines> lines;

  //Processed geometry ready for rendering
  bool updateVertexBuffer{true};
  std::vector<LinesDetails_lt> processedGeometry;
  float lineWidth{1.0f};

  //################################################################################################
  Private(Q* q_):
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
  setExcludeFromPicking(true);
}

//##################################################################################################
LinesLayer::~LinesLayer()
{
  delete d;
}

//##################################################################################################
const std::vector<Lines>& LinesLayer::lines() const
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
void LinesLayer::updateLines(const std::function<void(std::vector<Lines>&)>& closure)
{
  closure(d->lines);
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void LinesLayer::setLinesFromGeometry(const std::vector<tp_math_utils::Geometry3D>& geometry)
{
  std::vector<Lines> lines;

  for(const auto& g : geometry)
  {
    glm::vec3 albedo{0.0f, 0.0f, 0.0f};
    tp_math_utils::OpenGLMaterial::view(g.material,[&](const tp_math_utils::OpenGLMaterial& m)
    {
      albedo = m.albedo;
    });

    for(const auto& m : g.indexes)
    {
      auto& l = lines.emplace_back();
      l.color = glm::vec4(albedo, 1.0f);
      l.mode = GL_LINES;

      if(!m.indexes.empty())
      {
        auto getVert = [&](size_t ii)
        {
          return g.verts.at(size_t(m.indexes.at(ii))).vert;
        };

        switch(m.type)
        {
        case GL_TRIANGLE_FAN:
        {
          l.lines.reserve(m.indexes.size()*2 - 2);
          auto first = getVert(0);
          for(size_t i=1; i<m.indexes.size(); i++)
          {
            l.lines.push_back(first);
            l.lines.push_back(getVert(i));
          }
          break;
        }

        case GL_TRIANGLE_STRIP:
        {
          l.lines.reserve(m.indexes.size()*2 - 1);
          for(size_t i=1; i<m.indexes.size(); i++)
          {
            l.lines.push_back(getVert(i-1));
            l.lines.push_back(getVert(i));
          }

          for(size_t i=3; i<m.indexes.size(); i+=2)
          {
            l.lines.push_back(getVert(i-2));
            l.lines.push_back(getVert(i));
          }

          for(size_t i=2; i<m.indexes.size(); i+=2)
          {
            l.lines.push_back(getVert(i-2));
            l.lines.push_back(getVert(i));
          }

          break;
        }

        case GL_TRIANGLES:
        {
          l.lines.reserve(m.indexes.size()*2);
          for(size_t i=2; i<m.indexes.size(); i+=3)
          {
            l.lines.push_back(getVert(i-2));
            l.lines.push_back(getVert(i-1));

            l.lines.push_back(getVert(i-1));
            l.lines.push_back(getVert(i));

            l.lines.push_back(getVert(i));
            l.lines.push_back(getVert(i-2));
          }
          break;
        }
        }
      }
    }
  }

  setLines(lines);
}

//##################################################################################################
void LinesLayer::setLinesFromGeometryNormals(const std::vector<tp_math_utils::Geometry3D>& geometry, float scale)
{
  size_t vertCount=0;
  for(const auto& m : geometry)
    vertCount += m.verts.size();
  vertCount*=2;

  std::vector<tp_maps::Lines> lines;
  lines.resize(3);
  auto& r = lines.at(0);
  auto& g = lines.at(1);
  auto& b = lines.at(2);

  r.lines.reserve(vertCount);
  g.lines.reserve(vertCount);
  b.lines.reserve(vertCount);

  r.color = {1.0f, 0.0f, 0.0f, 1.0f};
  g.color = {0.0f, 1.0f, 0.0f, 1.0f};
  b.color = {0.0f, 0.0f, 1.0f, 1.0f};

  r.mode = GL_LINES;
  g.mode = GL_LINES;
  b.mode = GL_LINES;

  for(const auto& m : geometry)
  {
    for(const auto& v : m.verts)
    {
      r.lines.push_back(v.vert);
      g.lines.push_back(v.vert);
      b.lines.push_back(v.vert);

      b.lines.push_back(v.vert + (v.normal   * scale));
    }
  }

  setLines(lines);
}

//##################################################################################################
void LinesLayer::setLinesFromGeometryTangents(const std::vector<tp_math_utils::Geometry3D>& geometry, float scale)
{
  size_t vertCount=0;
  for(const auto& m : geometry)
    vertCount += m.verts.size();
  vertCount*=2;

  std::vector<tp_maps::Lines> lines;
  lines.resize(3);
  auto& r = lines.at(0);
  auto& g = lines.at(1);
  auto& b = lines.at(2);

  r.lines.reserve(vertCount);
  g.lines.reserve(vertCount);
  b.lines.reserve(vertCount);

  r.color = {1.0f, 0.0f, 0.0f, 1.0f};
  g.color = {0.0f, 1.0f, 0.0f, 1.0f};
  b.color = {0.0f, 0.0f, 1.0f, 1.0f};

  r.mode = GL_LINES;
  g.mode = GL_LINES;
  b.mode = GL_LINES;

  for(const auto& m : geometry)
  {
    std::vector<glm::vec3> tangent;
    m.buildTangentVectors(tangent);

    for(size_t i=0; i<tangent.size(); ++i)
    {
      const auto& v = m.verts.at(i);
      r.lines.push_back(v.vert);
      g.lines.push_back(v.vert);
      b.lines.push_back(v.vert);

      g.lines.push_back(v.vert + (tangent.at(i)   * scale));
    }
  }

  setLines(lines);
}

//##################################################################################################
float LinesLayer::lineWidth() const
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
  return map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();
}

//##################################################################################################
void LinesLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass().type &&
     renderInfo.pass != RenderPass::Picking)
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

  shader->use(renderInfo.shaderType());
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
  Layer::invalidateBuffers();
}

}
