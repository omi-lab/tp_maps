#ifndef tp_maps_EventHandler_h
#define tp_maps_EventHandler_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;
struct MouseEvent;
struct KeyEvent;
struct DragDropEvent;
struct TextEditingEvent;
struct TextInputEvent;

//##################################################################################################
struct TP_MAPS_EXPORT EventHandlerCallbacks
{
  std::function<bool(const MouseEvent&)>             mouseEvent{[](const auto&){return false;}};
  std::function<bool(const KeyEvent&)>                 keyEvent{[](const auto&){return false;}};
  std::function<bool(const DragDropEvent&)>       dragDropEvent{[](const auto&){return false;}};
  std::function<bool(const TextEditingEvent&)> textEditingEvent{[](const auto&){return false;}};
  std::function<bool(const TextInputEvent&)>     textInputEvent{[](const auto&){return false;}};
};

//##################################################################################################
class EventHandler
{
  TP_NONCOPYABLE(EventHandler);
public:
  //################################################################################################
  EventHandler(Map* map, int priority);

  //################################################################################################
  ~EventHandler();

  //################################################################################################
  int priority() const;

  //################################################################################################
  size_t id() const;

  //################################################################################################
  void updateCallbacks(const std::function<void(EventHandlerCallbacks&)>& closure);

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
