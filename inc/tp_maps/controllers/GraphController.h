#ifndef tp_maps_GraphController_h
#define tp_maps_GraphController_h

#include "tp_maps/Controller.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT GraphController : public Controller
{
  TP_DQ;
public:
  //################################################################################################
  GraphController(Map* map);

  //################################################################################################
  glm::dvec3 focalPoint() const;

  //################################################################################################
  void setFocalPoint(const glm::dvec3& focalPoint);

  //################################################################################################
  bool allowTranslation() const;

  //################################################################################################
  void setAllowTranslation(bool allowTranslation);

  //################################################################################################
  bool allowZoom() const;

  //################################################################################################
  void setAllowZoom(bool allowZoom);

  //################################################################################################
  double rotationFactor() const;

  //################################################################################################
  void setRotationFactor(double rotationFactor);

  //################################################################################################
  double distanceX() const;

  //################################################################################################
  void setDistanceX(double distanceX);

  //################################################################################################
  double distanceY() const;

  //################################################################################################
  void setDistanceY(double distanceY);

  //################################################################################################
  void saveState(nlohmann::json& j) const override;

  //################################################################################################
  void loadState(const nlohmann::json& j) override;

protected:
  //################################################################################################
  ~GraphController() override;

  //################################################################################################
   void mapResized(int w, int h) override;

  //################################################################################################
  void updateMatrices() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  virtual void translate(double dx, double dy, double msSincePrevious);

  //################################################################################################
  virtual void translateInteractionFinished();

  //################################################################################################
  virtual void translateInteractionStarted();
};

}

#endif
