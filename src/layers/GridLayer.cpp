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

  float scale;

  //################################################################################################
  Private(GridLayer* q_, float scale_):
    q(q_),
    scale(scale_)
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

      for(size_t a=0; a<3; a++)
      {
        glm::vec3 maskA{0,0,0};
        maskA[int(a)]=scale*0.01f;

        glm::vec3 maskB{0,0,0};
        maskB[int((a+1)%3)]=scale*0.1f;

        glm::vec3 maskC{0,0,0};
        maskC[int((a+2)%3)]=scale*0.1f;

        std::vector<glm::vec3> vertices;
        for(int i=0; i< 100; i++)
        {
          auto aa = float(i)*maskA;
          vertices.emplace_back(aa);
          vertices.emplace_back(aa+maskB);

          vertices.emplace_back(aa);
          vertices.emplace_back(aa+maskC);
        }

        LinesDetails_lt details;
        details.vertexBuffer = shader->generateVertexBuffer(q->map(), vertices);
        details.color = {0.0f, 0.0f, 0.0f};
        details.color[int(a)] = 1.0f;
        processedGeometry.push_back(details);
      }
    }

    shader->use();
    shader->setMatrix(matrix);
    shader->setLineWidth(1.0f);

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

    //FontShader::PreparedString someText(shader, font, u"Hello");
    //shader->drawPreparedString(someText);
  }
};

//##################################################################################################
GridLayer::GridLayer(float scale):
  d(new Private(this, scale))
{
}

//##################################################################################################
GridLayer::~GridLayer()
{
  delete d;
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
  d->renderText(matrix);
}

//##################################################################################################
void GridLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
