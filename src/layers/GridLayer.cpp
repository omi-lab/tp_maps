#include "tp_maps/layers/GridLayer.h"
#include "tp_maps/shaders/LineShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/Controller.h"

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

  //################################################################################################
  Private(GridLayer* q_):
    q(q_)
  {
    for(int i=0; i< 100; i++)
    {
      vertices.push_back(glm::vec3(float(i) * 0.1f,  0.0f, 0.0f));
      vertices.push_back(glm::vec3(float(i) * 0.1f,  4.0f, 0.0f));

      vertices.push_back(glm::vec3( 0.0f, float(i) * 0.1f, 0.0f));
      vertices.push_back(glm::vec3( 2.0f, float(i) * 0.1f, 0.0f));
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

  shader->use();
  shader->setMatrix(map()->controller()->matrix(coordinateSystem()));
  shader->setLineWidth(1.0f);
  shader->setColor({1.0f, 0.0f, 0.0f});

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
