#ifndef tp_maps_MouseEvent_h
#define tp_maps_MouseEvent_h

#include "tp_maps/Globals.h"

namespace tp_maps
{

//##################################################################################################
enum class MouseEventType
{
  Press,
  Move,
  Release,
  Wheel,
  DoubleClick,
  Click        //!< A single click and release, only used for Controller::setMouseClickCallback.
};

//##################################################################################################
enum class Button : size_t
{
  NoButton     = 0,
  RightButton  = 1,
  LeftButton   = 2,
  MiddleButton = 4
};

//##################################################################################################
struct TP_MAPS_EXPORT MouseEvent
{
  //! The type of this mouse event.
  MouseEventType type{MouseEventType::Press};

  //! The state of the mouse buttons associated with this event.
  Button button{Button::NoButton};

  //! The position of the mouse event.
  glm::ivec2 pos{0,0};

  //! Movement since last event, usually used for full screen.
  glm::ivec2 posDelta{0,0};

  //! Mouse wheel delta, where applicable.
  int delta{0};

  //! Are any modifier keys currently pressed.
  KeyboardModifier modifiers{KeyboardModifier::None};

  MouseEvent(MouseEventType type_=MouseEventType::Press):
    type(type_)
  {

  }
};

}

#endif
