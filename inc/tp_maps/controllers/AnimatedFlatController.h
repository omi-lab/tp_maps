#ifndef tp_maps_AnimatedFlatController_h
#define tp_maps_AnimatedFlatController_h

#include "tp_maps/controllers/FlatController.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT AnimatedFlatController : public FlatController
{
public:
  //################################################################################################
  AnimatedFlatController(Map* map);

  //################################################################################################
  void animationReset();

  //################################################################################################
  void animateToFocalPoint(const glm::vec3& focalPoint);

  //################################################################################################
  void animateToDistance(float distance);

  //################################################################################################
  void animateToRotationAngle(float rotationAngle);

protected:
  //################################################################################################
  ~AnimatedFlatController() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  void animate(double timestampMS) override;

  //################################################################################################
  void translate(float dx, float dy, double msSincePrevious) override;

  //################################################################################################
  void translateInteractionFinished() override;

  //################################################################################################
  void translateInteractionStarted() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
