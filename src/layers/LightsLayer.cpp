#include "tp_maps/layers/LightsLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/layers/FrustumLayer.h"
#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/shaders/FontShader.h"

#include "tp_math_utils/Sphere.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <vector>

namespace tp_maps
{

namespace
{
struct LabelDetails_lt
{
  glm::vec3 position;
  std::shared_ptr<tp_maps::FontShader::PreparedString> preparedString;
};
}

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

  FontRenderer* font{nullptr};
  bool regenerateText{true};
  std::vector<LabelDetails_lt> labels;
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
void LightsLayer::setFont(FontRenderer* font)
{
  d->regenerateText = false;
  d->font = font;
  update();
}

//##################################################################################################
FontRenderer* LightsLayer::font() const
{
  return d->font;
}

//##################################################################################################
void LightsLayer::render(RenderInfo& renderInfo)
{
  if(d->updateModels/* && renderInfo.pass == defaultRenderPass()*/)
  {
    d->updateModels = false;

    {
      auto bulb = tp_math_utils::Sphere::octahedralClass1(0.1f, 3, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES);
      std::vector<tp_math_utils::Geometry3D> geometry;
      for(const auto& light : map()->lights())
      {
        auto& g = geometry.emplace_back();
        g = bulb;
        glm::mat4 m{1.0f};
        m = glm::translate(m, light.position());
        g.transform(m);
      }
      d->bulbs->setLinesFromGeometry(geometry);
    }

    {
      std::vector<glm::mat4> lights;
      for(const auto& light : map()->lightBuffers())
        lights.push_back(light.worldToTexture.empty()?glm::mat4(1.0f):light.worldToTexture.front().vp);
      d->frustums->setCameraMatrices(lights);
    }

    {
      const auto& lights = map()->lights();
      while(d->gizmoLayers.size()<lights.size())
      {
        size_t i=d->gizmoLayers.size();
        auto gizmoLayer = new GizmoLayer();
        gizmoLayer->setEnableRotation(true, true, true);
        gizmoLayer->setEnableTranslation(true, true, true);
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

  //Draw the text.
  if(renderInfo.pass == RenderPass::Text && font())
  {
    auto shader = map()->getShader<tp_maps::FontShader>();
    if(shader->error())
      return;

    if(d->regenerateText)
    {
      d->regenerateText = false;
      tp_maps::PreparedStringConfig config;
      config.topDown = true;
      config.relativeOffset.x = 1.0f;
      config.pixelOffset.x = 5.0f;

      const auto& lights = map()->lights();
      d->labels.resize(lights.size());
      for(size_t l=0; l<lights.size(); l++)
      {
        const auto& light = lights.at(l);
        auto& label = d->labels.at(l);
        label.preparedString.reset(new tp_maps::FontShader::PreparedString(shader, font(), tpFromUTF8(light.name.keyString()), config));
        label.position = light.position();
      }

    }

    float width  = float(map()->width());
    float height = float(map()->height());

    glm::mat4 matrix{1.0f};
    matrix = glm::translate(matrix, {-1.0f, 1.0f, 0.0f});
    matrix = glm::scale(matrix, {2.0f/width, -2.0f/height, 1.0f});

    auto m = map()->controller()->matrix(tp_maps::defaultSID());

    shader->use();
    shader->setColor({0.0f, 0.0f, 0.0f, 1.0f});
    for(const auto& label : d->labels)
    {
      auto p = tpProj(m, label.position);
      if(std::fabs(p.z)>1.0f)
        continue;

      p.x = ((p.x+1.0f) / 2.0f) * width;
      p.y = (1.0f - ((p.y+1.0f) / 2.0f)) * height;

      shader->setMatrix(glm::translate(matrix, glm::floor(glm::vec3(std::floor(p.x), std::floor(p.y), 0.0f))));
      shader->drawPreparedString(*label.preparedString.get());
    }
  }

  Layer::render(renderInfo);
}

//##################################################################################################
void LightsLayer::lightsChanged(LightingModelChanged lightingModelChanged)
{
  TP_UNUSED(lightingModelChanged);
  d->updateModels = true;
  d->regenerateText = true;
}

}
