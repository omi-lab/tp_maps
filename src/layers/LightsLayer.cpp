#include "tp_maps/layers/LightsLayer.h"
#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/layers/FrustumLayer.h"
#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/shaders/FontShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/picking_results/LinesPickingResult.h"

#include "tp_math_utils/Sphere.h"

#include "glm/gtc/matrix_transform.hpp" // IWYU pragma: keep

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

  bool editLights;

  size_t lightIndex{0};

  LinesLayer* bulbs{nullptr};
  FrustumLayer* frustums{nullptr};

  std::vector<GizmoLayer*> gizmoLayers;

  FontRenderer* font{nullptr};
  std::vector<LabelDetails_lt> labels;

  size_t clickIndex{0};
  bool clickActive{false};
  bool mouseMoved{false};

  bool updateModels{true};
  bool updateFrustums{true};
  bool regenerateText{true};
  bool updateVisibility{true};
};

//##################################################################################################
LightsLayer::LightsLayer(bool editLights):
  d(new Private())
{
  d->editLights = editLights;

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
  d->regenerateText = true;
  d->font = font;
  update();
}

//##################################################################################################
FontRenderer* LightsLayer::font() const
{
  return d->font;
}

//##################################################################################################
void LightsLayer::setLightIndex(size_t lightIndex)
{
  d->lightIndex = lightIndex;
  d->updateVisibility = true;
  update();
}

//##################################################################################################
size_t LightsLayer::lightIndex() const
{
  return d->lightIndex;
}

//##################################################################################################
void LightsLayer::render(RenderInfo& renderInfo)
{
  if(d->updateModels)
  {
    d->updateModels = false;
    d->updateVisibility = true;

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

    if(d->editLights)
    {
      const auto& lights = map()->lights();
      while(d->gizmoLayers.size()<lights.size())
      {
        size_t i=d->gizmoLayers.size();
        auto gizmoLayer = new GizmoLayer();
        gizmoLayer->setEnableRotation(true, true, true);
        gizmoLayer->setEnableTranslation(true, true, true);
        gizmoLayer->setEnableScale(false, false, false);
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

  if(d->updateFrustums && size_t(renderInfo.pass.type) > size_t(RenderPass::LightFBOs))
  {
    d->updateFrustums = false;

    {
      std::vector<glm::mat4> lights;
      lights.reserve(map()->lightBuffers().size());
      for(const auto& light : map()->lightBuffers())
        lights.push_back(light.worldToTexture.empty()?glm::mat4(1.0f):light.worldToTexture.front().vp);
      d->frustums->setCameraMatrices(lights);
    }
  }

  if(d->updateVisibility)
  {
    for(size_t i=0; i<d->gizmoLayers.size(); i++)
      d->gizmoLayers.at(i)->setVisible(d->lightIndex == i);
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
        label.preparedString.reset(new tp_maps::FontShader::PreparedString(font(), tpFromUTF8(light.name.toString()), config));
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
    for(size_t i=0; i<d->labels.size(); i++)
    {
      // if(d->lightIndex != i)
      //   continue;

      const auto& label = d->labels.at(i);

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
bool LightsLayer::mouseEvent(const tp_maps::MouseEvent& event)
{
  if(tp_maps::Layer::mouseEvent(event))
    return true;

  switch(event.type)
  {
  case tp_maps::MouseEventType::Press: //-----------------------------------------------------------
  {
    if(event.button != tp_maps::Button::LeftButton)
      return false;

    tp_maps::PickingResult* pickingResult = map()->performPicking("LightsLayer", event.pos);
    TP_CLEANUP([&]{delete pickingResult;});
    if(!pickingResult)
    {
      d->clickActive = false;
      return false;
    }

    if(pickingResult->layer == d->bulbs || pickingResult->layer == d->frustums)
    {
      if(auto linesPickingResult = dynamic_cast<LinesPickingResult*>(pickingResult); linesPickingResult)
      {
        d->clickIndex = linesPickingResult->index;
        d->mouseMoved = false;
        d->clickActive = true;
        return true;
      }
    }

    return false;
  }

  case tp_maps::MouseEventType::Move: //------------------------------------------------------------
  {
    if(event.button != tp_maps::Button::LeftButton)
      return false;

    if(d->clickActive)
    {
      d->mouseMoved = true;
      return true;
    }
    return false;
  }

  case tp_maps::MouseEventType::Release: //---------------------------------------------------------
  {
    if(event.button != tp_maps::Button::LeftButton)
      return false;

    if(d->clickActive)
    {
      if(!d->mouseMoved)
      {
        setLightIndex(d->clickIndex);
        lightIndexChanged();
      }

      d->clickActive = false;
      return true;
    }

    return false;
  }

  default: //---------------------------------------------------------------------------------------
    break;
  }

  return false;
}

//##################################################################################################
void LightsLayer::lightsChanged(LightingModelChanged lightingModelChanged)
{
  TP_UNUSED(lightingModelChanged);
  d->updateModels = true;
  d->updateFrustums = true;
  d->regenerateText = true;
}

}
