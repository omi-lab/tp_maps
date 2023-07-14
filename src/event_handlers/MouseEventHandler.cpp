#include "tp_maps/event_handlers/MouseEventHandler.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

namespace
{

//##################################################################################################
struct EventHandler_lt
{
  EventHandler eventHandler;
  MouseEvent pressEvent;

  EventHandler_lt(Map* map):
    eventHandler(map, 1)
  {

  }
};
}

//##################################################################################################
struct MouseEventHandler::Private
{
  Map* map;

  std::unordered_map<Button, std::unique_ptr<EventHandler_lt>> eventHandlers;

  //################################################################################################
  Private(Map* map_):
    map(map_)
  {

  }

  //################################################################################################
  std::function<bool(const MouseEvent&)> mouseEvent = [&](const MouseEvent& event)
  {
    bool consumeEvent=false;

    for(auto i=eventHandlers.begin(); i!=eventHandlers.end(); )
    {
      const std::unique_ptr<EventHandler_lt>& eventHandler = i->second;

      bool incrementIterator=true;
      TP_CLEANUP([&]
      {
        if(incrementIterator)
          ++i;
      });

      auto eraseHandler = [&]
      {
        incrementIterator = false;
        i = eventHandlers.erase(i);
      };

      switch(event.type)
      {
      case MouseEventType::Move:
      {
        auto t = glm::abs(eventHandler->pressEvent.pos - event.pos);
        if((t.x+t.y) > 3)
        {
          MouseEvent e(MouseEventType::DragStart);
          e.button = eventHandler->pressEvent.button;
          e.pos = eventHandler->pressEvent.pos;
          e.modifiers = eventHandler->pressEvent.modifiers;
          eraseHandler();
          map->mouseEvent(e);
        }

        consumeEvent = true;
        break;
      }

      case MouseEventType::Release:
      {
        MouseEvent e(MouseEventType::Click);
        e.button = eventHandler->pressEvent.button;
        e.pos = eventHandler->pressEvent.pos;
        e.modifiers = eventHandler->pressEvent.modifiers;
        eraseHandler();
        map->mouseEvent(e);

        consumeEvent = true;
        break;
      }

      default:
      break;
      }
    }

    return consumeEvent;
  };
};

//##################################################################################################
MouseEventHandler::MouseEventHandler(Map* map):
  d(new Private(map))
{

}

//##################################################################################################
MouseEventHandler::~MouseEventHandler()
{
  delete d;
}

//##################################################################################################
size_t MouseEventHandler::press(const MouseEvent& event)
{
  auto& eventHandler = d->eventHandlers[event.button];

  eventHandler.reset(new EventHandler_lt(d->map));
  eventHandler->pressEvent = event;
  eventHandler->eventHandler.updateCallbacks([this](EventHandlerCallbacks& callbacks)
  {
    callbacks.mouseEvent = d->mouseEvent;
  });

  return eventHandler->eventHandler.id();
}

}
