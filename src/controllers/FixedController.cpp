#include "tp_maps/controllers/FixedController.h"

namespace tp_maps
{

//##################################################################################################
FixedController::FixedController(Map* map):
  Controller(map)
{
}

//##################################################################################################
void FixedController::saveState(nlohmann::json& j) const
{
  TP_UNUSED(j);
}

//##################################################################################################
void FixedController::loadState(const nlohmann::json& j)
{
  TP_UNUSED(j);
}

//##################################################################################################
void FixedController::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
}

//##################################################################################################
void FixedController::updateMatrices()
{

}

//##################################################################################################
bool FixedController::mouseEvent(const MouseEvent& event)
{
  TP_UNUSED(event);
  return false;
}

}
