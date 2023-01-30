#ifndef tp_maps_KeyEvent_h
#define tp_maps_KeyEvent_h

#include "tp_maps/Globals.h"

namespace tp_maps
{

//##################################################################################################
enum class KeyEventType
{
  Press,
  Release
};

//##################################################################################################
struct TP_MAPS_EXPORT KeyEvent
{
  //! The type of this key event.
  KeyEventType type{KeyEventType::Press};

  int32_t scancode{0};

  //! Are any modifier keys currently pressed.
  KeyboardModifier modifiers{KeyboardModifier::None};

  KeyEvent(KeyEventType type_=KeyEventType::Press):
    type(type_)
  {

  }
};

//##################################################################################################
//! Used for the current state of editing.
struct TP_MAPS_EXPORT TextEditingEvent
{
  std::string text;      //!< The text being edited.
  int cursor{0};         //!< The current cursor position or start of selection.
  int selectionLength{0}; //!< The length of the selection from the cursor position.
};

//##################################################################################################
//! Used for new text received from the user.
struct TP_MAPS_EXPORT TextInputEvent
{
  std::string text;
};

}

#endif
