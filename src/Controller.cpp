#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"
#include "tp_maps/Subview.h"

#include "glm/gtx/transform.hpp" // IWYU pragma: keep

#include <unordered_map>

namespace tp_maps
{

//##################################################################################################
struct Controller::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Controller::Private");
  TP_NONCOPYABLE(Private);

  Map* map;
  std::unordered_map<tp_utils::StringID, Matrices> matrices;
  std::unordered_map<tp_utils::StringID, Scissor> scissor;

  tp_math_utils::Light currentLight;
  Matrices lightMatrices;

  //################################################################################################
  Private(Map* map_):
    map(map_)
  {

  }
};

//##################################################################################################
Controller::Controller(Map* map):
  d(new Private(map))
{
  map->setController(this);
}

//##################################################################################################
glm::mat4 Controller::matrix(const tp_utils::StringID& coordinateSystem) const
{
  return tpGetMapValue(d->matrices, coordinateSystem).vp;
}

//##################################################################################################
Matrices Controller::matrices(const tp_utils::StringID& coordinateSystem) const
{
  return tpGetMapValue(d->matrices, coordinateSystem);
}

//##################################################################################################
bool Controller::hasMatrices(const tp_utils::StringID& coordinateSystem) const
{
  return tpContainsKey(d->matrices, coordinateSystem);
}

//##################################################################################################
void Controller::setCurrentLight(const tp_math_utils::Light& light)
{
  d->currentLight = light;

  float distance = light.orthoRadius;

  //glm::mat4 view = glm::lookAt(d->currentLight.position, d->currentLight.position + d->currentLight.direction, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 view = d->currentLight.viewMatrix;

  glm::mat4 projection;

  switch(light.type)
  {
  case tp_math_utils::LightType::Global: [[fallthrough]];
  case tp_math_utils::LightType::Directional:
  {
    projection = glm::ortho(-distance,  // <- Left
                            distance,   // <- Right
                            -distance,  // <- Bottom
                            distance,   // <- Top
                            light.near, // <- Near
                            light.far); // <- Far
    break;
  }

  case tp_math_utils::LightType::Spot:
  {
    projection = glm::perspective(glm::radians(light.fov), 1.0f, light.near, light.far);
    break;
  }
  }

  Matrices vp;
  vp.p  = projection;
  vp.v  = view;
  vp.vp = projection * view;

  d->lightMatrices = vp;
}

//##################################################################################################
Matrices Controller::lightMatrices()
{
  return d->lightMatrices;
}

//##################################################################################################
Controller::Scissor Controller::scissor(const tp_utils::StringID& coordinateSystem) const
{
  return tpGetMapValue(d->scissor, coordinateSystem, Controller::Scissor());
}

//##################################################################################################
void Controller::enableScissor(const tp_utils::StringID& coordinateSystem)
{
  Scissor s = d->scissor[coordinateSystem];
  if(!s.valid)
    return;

  glEnable(GL_SCISSOR_TEST);
  glScissor(s.x, s.y, s.width, s.height);
}

//##################################################################################################
void Controller::disableScissor()
{
  glDisable(GL_SCISSOR_TEST);
}

//##################################################################################################
void Controller::saveState(nlohmann::json& j) const
{
  TP_UNUSED(j);
}

//##################################################################################################
void Controller::loadState(const nlohmann::json& j)
{
  TP_UNUSED(j);
}

//##################################################################################################
Map* Controller::map() const
{
  return d->map;
}

//##################################################################################################
Subview* Controller::subview() const
{
  for(auto subview : d->map->allSubviews())
    if(subview->controller() == this)
      return subview;

  return d->map->currentSubview();
}

//##################################################################################################
Controller::~Controller()
{
  delete d;
}

//##################################################################################################
void Controller::update(RenderFromStage renderFromStage)
{
  if(d->map)
  {
    d->map->controllerUpdate();
    d->map->update(renderFromStage, this);
  }
}

//##################################################################################################
void Controller::setMatrix(const tp_utils::StringID& coordinateSystem, const glm::mat4& matrix)
{
  d->matrices[coordinateSystem].vp = matrix;
}

//##################################################################################################
void Controller::setMatrices(const tp_utils::StringID& coordinateSystem, const Matrices& matrices)
{
  d->matrices[coordinateSystem] = matrices;
}

//##################################################################################################
void Controller::setScissor(const tp_utils::StringID& coordinateSystem, int x, int y, int width, int height)
{
  Scissor& s = d->scissor[coordinateSystem];
  s.valid = true;
  s.x = x;
  s.y = y;
  s.width = width;
  s.height = height;
}

//##################################################################################################
void Controller::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
}

//##################################################################################################
bool Controller::mouseEvent(const MouseEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
bool Controller::keyEvent(const KeyEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
void Controller::animate(double timestampMS)
{
  TP_UNUSED(timestampMS);
}

}
