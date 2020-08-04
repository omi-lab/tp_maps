#include "tp_maps/layers/LightsLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/layers/FrustumLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"

#include "tp_math_utils/Sphere.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct LightsLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::LightsLayer::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  LinesLayer* bulbs{nullptr};
  FrustumLayer* frustums{nullptr};

  bool updateModels{true};
};

//##################################################################################################
LightsLayer::LightsLayer():
  d(new Private())
{
  d->bulbs = new tp_maps::LinesLayer();
  addChildLayer(d->bulbs);

  d->frustums = new tp_maps::FrustumLayer();
  addChildLayer(d->frustums);
}

//##################################################################################################
LightsLayer::~LightsLayer()
{
  delete d;
}

//##################################################################################################
void LightsLayer::render(RenderInfo& renderInfo)
{
  if(d->updateModels && renderInfo.pass == defaultRenderPass())
  {
    d->updateModels = false;

    {
      auto bulb = tp_math_utils::Sphere::octahedralClass1(0.1f, 3, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES);
      std::vector<tp_maps::Geometry3D> geometry;
      for(const auto& light : map()->lights())
      {
        auto& g = geometry.emplace_back();
        g.geometry = bulb;
        glm::mat4 m{1.0f};
        m = glm::translate(m, light.position);
        g.geometry.transform(m);
      }
      d->bulbs->setLinesFromGeometry(geometry);
    }

    {
      std::vector<glm::mat4> lights;
      for(const auto& light : map()->lightTextures())
        lights.push_back(light.worldToTexture);
      d->frustums->setCameraMatrices(lights);
    }
  }

  Layer::render(renderInfo);
}

//##################################################################################################
void LightsLayer::lightsChanged(LightingModelChanged lightingModelChanged)
{
  TP_UNUSED(lightingModelChanged);
  d->updateModels = true;
}

}
