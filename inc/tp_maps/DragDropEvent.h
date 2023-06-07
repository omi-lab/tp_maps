#ifndef tp_maps_DragDropEvent_h
#define tp_maps_DragDropEvent_h

#include "tp_maps/Globals.h"
#include "json.hpp"

namespace tp_maps
{

//##################################################################################################
enum class DragDropEventType
{
  None,
  Drop,  //!< Event which is sent when a drag and drop action is completed
  Enter, //!< Event which is sent to a widget when a drag and drop action enters it.
  Move,  //!< Event which is sent while a drag and drop action is in progress.
  Leave, //!< Event that is sent to a widget when a drag and drop action leaves it
};

//##################################################################################################
struct TP_MAPS_EXPORT DragDropEvent
{
  //! The type of this mouse event.
  DragDropEventType type{DragDropEventType::None};

  //! The position of mouse during DragDropEventType::Drop, DragDropEventType::Move event.
  glm::ivec2 pos{0,0};
  //! Event Metadata
  nlohmann::json payload;

  DragDropEvent(DragDropEventType type_=DragDropEventType::None):
    type(type_)
  {
  }
};

}

#endif
