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
  j["fillColor"     ] = tp_math_utils::vec4ToJSON(fillColor     );
  j["borderColor"   ] = tp_math_utils::vec4ToJSON(borderColor   );
  j["startLineColor"] = tp_math_utils::vec4ToJSON(startLineColor);
  j["endLineColor"  ] = tp_math_utils::vec4ToJSON(endLineColor  );
}

//##################################################################################################
void CircleSectorParameters::loadState(const nlohmann::json& j)
{
  fillColor      = tp_math_utils::getJSONVec4(j, "fillColor"     , {1.0f, 0.0f, 0.0f, 1.0f});
  borderColor    = tp_math_utils::getJSONVec4(j, "borderColor"   , {1.0f, 0.0f, 0.0f, 1.0f});
  startLineColor = tp_math_utils::getJSONVec4(j, "startLineColor", {1.0f, 0.0f, 0.0f, 1.0f});
  endLineColor   = tp_math_utils::getJSONVec4(j, "endLineColor"  , {1.0f, 0.0f, 0.0f, 1.0f});
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
    lines.resize(3);

    std::vector<tp_math_utils::Geometry> geometry;
    geometry.resize(2);

    bool negative = d->angleDegrees<0.0f;

    float angleFabs = std::fabs(d->angleDegrees);

    size_t angleInt = angleFabs;

    if(angleInt>0)
      if((angleFabs-float(angleInt)) < 0.01f)
        angleInt--;

    if(angleFabs>0.001f)
    {
      auto& l = lines.at(0);
      auto& g = geometry.at(0);

      l.color = d->params.borderColor;
      l.mode = GL_LINE_LOOP;

      g.material.findOrAddOpenGL()->albedo = d->params.fillColor;

      l.lines.reserve(angleInt+2);

      auto addPoint = [&](float x, float y)
      {
        l.lines.emplace_back(x, y, 0.0f);
        g.geometry.emplace_back(x, y);
      };

      auto addAngle = [&](float a)
      {
        auto r = glm::radians(negative?a:-a);
        addPoint(std::cos(r), std::sin(r));
      };

      addPoint(0.0f, 0.0f);

      for(size_t i=0; i<angleInt; i++)
        addAngle(float(i));

      addAngle(angleFabs);
    }

    d->linesLayer->setLines(lines);
    d->geometryLayer->setGeometry(geometry);
  }

  Layer::render(renderInfo);
}

}
