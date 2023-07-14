#include "tp_maps/EventHandler.h"
#include "tp_maps/Map.h"

namespace tp_maps
{
class Map;
struct MouseEvent;
struct KeyEvent;
struct DragDropEvent;
struct TextEditingEvent;
struct TextInputEvent;

//##################################################################################################
struct EventHandler::Private
{
  Map* map;
  int priority;
  size_t id{0};

  //################################################################################################
  Private(Map* map_, int priority_):
    map(map_),
    priority(priority_),
    id(map->addEventHandler(priority))
  {

  }
};

//##################################################################################################
EventHandler::EventHandler(Map* map, int priority):
  d(new Private(map, priority))
{

}

//##################################################################################################
EventHandler::~EventHandler()
{
  d->map->removeEventHandler(d->id);
  delete d;
}

//##################################################################################################
int EventHandler::priority() const
{
  return d->priority;
}

//##################################################################################################
size_t EventHandler::id() const
{
  return d->id;
}

//##################################################################################################
void EventHandler::updateCallbacks(const std::function<void(EventHandlerCallbacks&)>& closure)
{
  d->map->updateEventHandlerCallbacks(d->id, closure);
}

}
