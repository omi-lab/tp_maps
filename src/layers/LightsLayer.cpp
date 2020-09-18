#include "tp_maps/layers/LightsLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/layers/FrustumLayer.h"
#include "tp_maps/layers/GizmoLayer.h"
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

  std::vector<GizmoLayer*> gizmoLayers;

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
        m = glm::translate(m, light.position());
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

    {
      const auto& lights = map()->lights();
      while(d->gizmoLayers.size()<lights.size())
      {
        size_t i=d->gizmoLayers.size();
        auto gizmoLayer = new GizmoLayer();
        gizmoLayer->setEnableRotation(true, true, true);
        gizmoLayer->setEnableTranslation(false, false, false);
        d->gizmoLayers.push_back(gizmoLayer);
        addChildLayer(gizmoLayer);

        gizmoLayer->changed.addCallback([&, i]
        {
          auto lights = map()->lights();
          if(i<lights.size() && i<d->gizmoLayers.size())
          {
            auto gizmoLayer = d->gizmoLayers.at(i);
            lights.at(i).viewMatrix = glm::inverse(gizmoLayer->modelMatrix());
            map()->setLights(lights);            
            lightsEdited();
          }
        });
      }

      while(d->gizmoLayers.size()>lights.size())
        delete tpTakeLast(d->gizmoLayers);

      for(size_t i=0; i<lights.size(); i++)
      {
        const auto& light = lights.at(i);
        auto gizmoLayer = d->gizmoLayers.at(i);
        gizmoLayer->setModelMatrix(glm::inverse(light.viewMatrix));
      }
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
