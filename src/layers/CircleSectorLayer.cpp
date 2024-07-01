#include "tp_maps/layers/CircleSectorLayer.h"
#include "tp_maps/layers/GeometryLayer.h"
#include "tp_maps/layers/LinesLayer.h"

#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/JSONUtils.h"

#include "json.hpp"

namespace tp_maps
{

//##################################################################################################
void CircleSectorParameters::saveState(nlohmann::json& j) const
{
  j["activeFillColor"  ] = tp_math_utils::vec4ToJSON(activeFillColor  );
  j["inactiveFillColor"] = tp_math_utils::vec4ToJSON(inactiveFillColor);
  j["borderColor"      ] = tp_math_utils::vec4ToJSON(borderColor      );
  j["startLineColor"   ] = tp_math_utils::vec4ToJSON(startLineColor   );
  j["endLineColor"     ] = tp_math_utils::vec4ToJSON(endLineColor     );
}

//##################################################################################################
void CircleSectorParameters::loadState(const nlohmann::json& j)
{
  activeFillColor   = tp_math_utils::getJSONVec4(j, "activeFillColor"  , {1.0f, 0.0f, 0.0f, 1.0f});
  inactiveFillColor = tp_math_utils::getJSONVec4(j, "inactiveFillColor", {1.0f, 0.0f, 0.0f, 1.0f});
  borderColor       = tp_math_utils::getJSONVec4(j, "borderColor"      , {1.0f, 0.0f, 0.0f, 1.0f});
  startLineColor    = tp_math_utils::getJSONVec4(j, "startLineColor"   , {1.0f, 0.0f, 0.0f, 1.0f});
  endLineColor      = tp_math_utils::getJSONVec4(j, "endLineColor"     , {1.0f, 0.0f, 0.0f, 1.0f});
}

//##################################################################################################
struct CircleSectorLayer::Private
{
  CircleSectorParameters params;
  float angleDegrees{45.0f};
  bool updateGeometry{true};

  GeometryLayer* geometryLayer{nullptr};
  LinesLayer* linesLayer{nullptr};
};

//##################################################################################################
CircleSectorLayer::CircleSectorLayer():
  d(new Private)
{  
  d->geometryLayer = new GeometryLayer();
  d->geometryLayer->setDrawBackFaces(true);
  addChildLayer(d->geometryLayer);

  d->linesLayer = new LinesLayer();
  addChildLayer(d->linesLayer);
}

//##################################################################################################
CircleSectorLayer::~CircleSectorLayer()
{
  delete d;
}

//##################################################################################################
void CircleSectorLayer::setParameters(const CircleSectorParameters& params)
{
  d->params = params;
  d->updateGeometry = true;
  update();
}

//##################################################################################################
const CircleSectorParameters& CircleSectorLayer::parameters() const
{
  return d->params;
}

//##################################################################################################
void CircleSectorLayer::setAngleDegrees(float angleDegrees)
{
  d->angleDegrees = angleDegrees;

  if(std::isnan(d->angleDegrees) || std::isinf(d->angleDegrees))
    d->angleDegrees = 0.0f;

  d->updateGeometry = true;
  update();
}

//##################################################################################################
float CircleSectorLayer::angleDegrees() const
{
  return d->angleDegrees;
}

//##################################################################################################
void CircleSectorLayer::setDefaultRenderPass(const RenderPass& defaultRenderPass)
{
  d->linesLayer->setDefaultRenderPass(defaultRenderPass);
  d->geometryLayer->setDefaultRenderPass(defaultRenderPass);
  Layer::setDefaultRenderPass(defaultRenderPass);
}

//##################################################################################################
void CircleSectorLayer::render(RenderInfo& renderInfo)
{
  if(d->updateGeometry && renderInfo.pass == RenderPass::PreRender)
  {
    d->updateGeometry = false;

    std::vector<Lines> lines;
    lines.resize(4);

    std::vector<tp_math_utils::Geometry> geometry;
    geometry.resize(2);

    bool negative = d->angleDegrees<0.0f;

    float angleFabs = std::fabs(d->angleDegrees);

    size_t angleInt = size_t(angleFabs);

    if(angleInt>0)
      if((angleFabs-float(angleInt)) < 0.01f)
        angleInt--;

    if(angleFabs>0.001f)
    {
      Lines* l{nullptr};
      tp_math_utils::Geometry* g{nullptr};

      auto configureArea = [&](size_t i, size_t n, const glm::vec4& borderColor, const glm::vec4& fillColor, bool drawOutline)
      {
        assert(i<geometry.size());

        if(drawOutline)
        {
          l = &lines.at(i);
          l->lines.reserve(n);
          l->color = borderColor;
          l->mode = GL_LINE_LOOP;
        }
        else
          l=nullptr;

        g = &geometry.at(i);
        g->geometry.reserve(n);
        g->material.findOrAddOpenGL()->albedo = fillColor;
        g->material.findOrAddOpenGL()->alpha = fillColor.w;
      };

      auto configureLine = [&](size_t i, const glm::vec4& borderColor)
      {
        assert(i<lines.size());
        l = &lines.at(i);
        l->lines.reserve(2);
        l->color = borderColor;
        l->mode = GL_LINES;

        g=nullptr;
      };

      auto addPoint = [&](float x, float y)
      {
        if(l)
          l->lines.emplace_back(x, y, 0.0f);

        if(g)
          g->geometry.emplace_back(x, y);
      };

      auto addAngle = [&](float a)
      {
        auto r = glm::radians(negative?a:-a);
        addPoint(std::cos(r), std::sin(r));
      };

      if(angleFabs>0.001f)
      {
        configureArea(0, angleInt+2, d->params.borderColor, d->params.activeFillColor, d->params.drawActiveOutline);

        addPoint(0.0f, 0.0f);

        for(size_t i=0; i<angleInt; i++)
          addAngle(float(i));

        addAngle(angleFabs);
      }

      if(angleFabs<359.999f)
      {
        configureArea(1, (360-angleInt)+2, d->params.borderColor, d->params.inactiveFillColor, d->params.drawInactiveOutline);

        addPoint(0.0f, 0.0f);

        addAngle(angleFabs);

        for(size_t i=angleInt+1; i<=360; i++)
          addAngle(float(i));
      }

      if(d->params.drawStartLine)
      {
        configureLine(2, d->params.startLineColor);
        addPoint(0.0f, 0.0f);
        addAngle(0.0f);
      }

      if(d->params.drawEndLine)
      {
        configureLine(3, d->params.endLineColor);
        addPoint(0.0f, 0.0f);
        addAngle(angleFabs);
      }
    }

    d->linesLayer->setLines(lines);
    d->geometryLayer->setGeometry(geometry);
  }

  Layer::render(renderInfo);
}

}
