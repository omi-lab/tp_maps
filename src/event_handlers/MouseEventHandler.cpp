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

  EventHandler_lt(Map* map, Button button):
    eventHandler(map, 1, button)
  {

  }
};
}

//##################################################################################################
struct MouseEventHandler::Private
{
  MouseEventHandler* q;
  Map* map;

  std::unordered_map<Button, std::unique_ptr<EventHandler_lt>> eventHandlers;

  //################################################################################################
  Private(MouseEventHandler* q_, Map* map_):
    q(q_),
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
          q->postMouseEvent(e);
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
        q->postMouseEvent(e);

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
  d(new Private(this, map))
{
  postMouseEvent = [&](const MouseEvent& event)
  {
    d->map->mouseEvent(event);
  };
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

  eventHandler.reset(new EventHandler_lt(d->map, event.button));
  eventHandler->pressEvent = event;
  eventHandler->eventHandler.updateCallbacks([this](EventHandlerCallbacks& callbacks)
  {
    callbacks.mouseEvent = d->mouseEvent;
  });

  return eventHandler->eventHandler.id();
}

}
