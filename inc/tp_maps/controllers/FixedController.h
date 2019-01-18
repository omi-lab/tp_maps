#ifndef tp_maps_FixedController_h
#define tp_maps_FixedController_h

#include "tp_maps/Controller.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT FixedController : public Controller
{
public:
  //################################################################################################
  FixedController(Map* map);

  //################################################################################################
  nlohmann::json saveState()const override;

  //################################################################################################
  void loadState(const nlohmann::json& j) override;

  using Controller::setMatrix;
  using Controller::setMatrices;
  using Controller::setScissor;

protected:
  //################################################################################################
  void mapResized(int w, int h) override;

  //################################################################################################
  void updateMatrices() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;
};

}

#endif
