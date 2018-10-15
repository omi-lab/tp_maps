#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/Controller.h"

#include "tp_utils/DebugUtils.h"

#include <vector>

namespace tp_maps
{
namespace
{
struct LinesDetails_lt
{
  LineShader::VertexBuffer* vertexBuffer{nullptr};
  glm::vec3 color;
};
}

//##################################################################################################
struct GridLayer::Private
{
  GridLayer* q;
  std::vector<glm::vec3> vertices;

  //Processed geometry ready for rendering
  std::vector<LinesDetails_lt> processedGeometry;
  bool updateVertexBuffer{true};

  float alpha{1.0f};

  //################################################################################################
  Private(GridLayer* q_):
    q(q_)
  {
    for(int i=0; i< 100; i++)
    {
      vertices.push_back(glm::vec3(float(i) * 0.001f,  0.0f, 0.0f));
      vertices.push_back(glm::vec3(float(i) * 0.001f,  0.4f, 0.0f));

      vertices.push_back(glm::vec3( 0.0f, float(i) * 0.001f, 0.0f));
      vertices.push_back(glm::vec3( 0.2f, float(i) * 0.001f, 0.0f));
    }
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
  void calculateGrid(const glm::mat4& matrix_)
  {
    auto matrix = glm::inverse(matrix_);
    glm::vec3 gridNormal(0.0f, 0.0f, 1.0f);

    float perpendicular{0.0f};
    {
      auto n = matrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
      auto f = matrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
      auto v = glm::vec3((n/n.w)-(f/f.w));

      perpendicular = glm::dot(gridNormal, glm::normalize(v));

      perpendicular = tpMax(std::fabs(perpendicular)*2.0f-1.0f, 0.0f);
    }

    alpha = 1.0f;//perpendicular;
  }
};

//##################################################################################################
GridLayer::GridLayer():
  d(new Private(this))
{
}

//##################################################################################################
GridLayer::~GridLayer()
{
  delete d;
}

//##################################################################################################
void GridLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != NormalRenderPass)
    return;

  LineShader* shader = map()->getShader<LineShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

      LinesDetails_lt details;
      details.vertexBuffer = shader->generateVertexBuffer(map(), d->vertices);
      details.color = {1.0f, 0.0f, 0.0f};
      d->processedGeometry.push_back(details);
  }

  glm::mat4 matrix = map()->controller()->matrix(coordinateSystem());

  d->calculateGrid(matrix);

  shader->use();
  shader->setMatrix(matrix);
  shader->setLineWidth(1.0f);
  shader->setColor({1.0f, 0.0f, 0.0f, d->alpha});

  map()->controller()->enableScissor(coordinateSystem());
  for(const LinesDetails_lt& line : d->processedGeometry)
    shader->drawLines(GL_LINES, line.vertexBuffer);
  map()->controller()->disableScissor();
}

//##################################################################################################
void GridLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
