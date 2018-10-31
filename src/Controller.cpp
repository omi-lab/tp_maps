#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"

#include <unordered_map>

namespace tp_maps
{

//##################################################################################################
struct Controller::Private
{
  Map* map;
  std::unordered_map<tp_utils::StringID, glm::mat4> matrices;
  std::unordered_map<tp_utils::StringID, Scissor> scissor;
  std::function<void(const MouseEvent&)> mouseClickCallback;

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
glm::mat4 Controller::matrix(const tp_utils::StringID& coordinateSystem)const
{
  return tpGetMapValue(d->matrices, coordinateSystem, glm::mat4(1));
}

//##################################################################################################
Controller::Scissor Controller::scissor(const tp_utils::StringID& coordinateSystem)const
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
Map* Controller::map()const
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
  d->matrices[coordinateSystem] = matrix;
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
