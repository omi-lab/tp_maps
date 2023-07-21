#ifndef tp_maps_MouseEventHandler_h
#define tp_maps_MouseEventHandler_h

#include "tp_maps/EventHandler.h"

#include <memory>

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT MouseEventHandler
{
public:
  //################################################################################################
  MouseEventHandler(Map* map);

  //################################################################################################
  ~MouseEventHandler();

  //################################################################################################
  size_t press(const MouseEvent& event);

  //################################################################################################
  std::function<void(const MouseEvent&)> postMouseEvent;

private:
  struct Private;
  friend struct Private;
  Private* d;
};
}

#endif
