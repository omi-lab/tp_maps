#ifndef tp_maps_KeyEvent_h
#define tp_maps_KeyEvent_h

#include "tp_maps/Globals.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
enum class KeyEventType
{
  Press,
  Release
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT KeyEvent
{
  //! The type of this key event.
  KeyEventType type{KeyEventType::Press};

  int32_t scancode{0};

  KeyEvent(KeyEventType type_=KeyEventType::Press):
    type(type_)
  {

  }
};

}

#endif
