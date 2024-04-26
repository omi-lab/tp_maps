#ifndef tp_maps_CADController_h
#define tp_maps_CADController_h

#include "tp_maps/Controller.h"

#include "tp_utils/CallbackCollection.h"

#pragma push_macro("far")
#undef far
#pragma push_macro("near")
#undef near


namespace tp_maps
{

//##################################################################################################
enum class CADControllerMode
{
  Perspective,
  OrthoXZ,
  OrthoYZ,
  OrthoXY
};

//##################################################################################################
std::vector<std::string> cadControllerModes();

//##################################################################################################
std::string cadControllerModeToString(CADControllerMode mode);

//##################################################################################################
CADControllerMode cadControllerModeFromString(const std::string& mode);

//##################################################################################################
class TP_MAPS_EXPORT CADController : public Controller
{
public:
  //################################################################################################
  CADController(Map* map, bool fullScreen);

  //################################################################################################
  CADControllerMode mode() const;

  //################################################################################################
  void setMode(CADControllerMode mode);

  //################################################################################################
  //! Multiplied with mouse translation and rotation operations
  void setMouseSpeedModifier(float mouseSpeedModifier);

  //################################################################################################
  float mouseSpeedModifier() const;

  //################################################################################################
  //! Multiplied with keyboard translation and rotation operations
  void setKeyboardSpeedModifier(float keyboardSpeedModifier);

  //################################################################################################
  float keyboardSpeedModifier() const;

  //################################################################################################
  glm::vec3 cameraOrigin() const;

  //################################################################################################
  void setCameraOrigin(const glm::vec3& cameraOrigin);

  //################################################################################################
  bool allowRotation() const;

  //################################################################################################
  void setAllowRotation(bool allowRotation);

  //################################################################################################
  bool variableViewAngle() const;

  //################################################################################################
  void setVariableViewAngle(bool variableViewAngle);

  //################################################################################################
  bool allowTranslation() const;

  //################################################################################################
  void setAllowTranslation(bool allowTranslation);

  //################################################################################################
  //! Rotation in degrees
  float rotationAngle() const;

  //################################################################################################
  void setRotationAngle(float rotationAngle);

  //################################################################################################
  //! Roll in degrees
  float rollAngle() const;

  //################################################################################################
  void setRollAngle(float rollAngle);

  //################################################################################################
  void setNearAndFar(float near, float far);

  //################################################################################################
  float near() const;

  //################################################################################################
  float far() const;

  //################################################################################################
  void setFOV(float fov);

  //################################################################################################
  float fov() const;

  //################################################################################################
  //! Calculate the forward vector of the camera.
  glm::vec3 forward() const;

  //################################################################################################
  //! Calculate the up vector of the camera.
  glm::vec3 up() const;

  //################################################################################################
  void setOrientation(const glm::vec3& forward, const glm::vec3& up);

  //################################################################################################
  nlohmann::json saveState() const override;

  //################################################################################################
  void loadState(const nlohmann::json& j) override;

  //################################################################################################
  void copyState(const CADController& other);

  //################################################################################################
  tp_utils::CallbackCollection<void()> userInteraction;

protected:
  //################################################################################################
  ~CADController() override;

  //################################################################################################
   void mapResized(int w, int h) override;

  //################################################################################################
  void updateMatrices() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  bool keyEvent(const KeyEvent& event) override;

  //################################################################################################
  void animate(double timestampMS) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#pragma pop_macro("far")
#pragma pop_macro("near")

#endif
