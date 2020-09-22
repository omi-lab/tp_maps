#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"

#include "glm/gtx/transform.hpp"

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
  std::function<void(const MouseEvent&)> mouseClickCallback;

  Light currentLight;
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
void Controller::setCurrentLight(const Light& light)
{
  d->currentLight = light;

  float distance = light.orthoRadius;

  //glm::mat4 view = glm::lookAt(d->currentLight.position, d->currentLight.position + d->currentLight.direction, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 view = d->currentLight.viewMatrix;

  glm::mat4 projection;

  switch(light.type)
  {
  case LightType::Global: [[fallthrough]];
  case LightType::Directional:
  {
    projection = glm::ortho(-distance,  // <- Left
                            distance,   // <- Right
                            -distance,  // <- Bottom
                            distance,   // <- Top
                            light.near, // <- Near
                            light.far); // <- Far
    break;
  }

  case LightType::Spot:
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
void Controller::setMouseClickCallback(const std::function<void(const MouseEvent&)>& mouseClickCallback)
{
  d->mouseClickCallback = mouseClickCallback;
}

//##################################################################################################
Map* Controller::map() const
{
  return d->map;
}

//##################################################################################################
Controller::~Controller()
{
  delete d;
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

//##################################################################################################
void Controller::callMouseClickCallback(const MouseEvent& event) const
{
  if(d->mouseClickCallback)
    d->mouseClickCallback(event);
}

}
