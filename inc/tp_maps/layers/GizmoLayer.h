#ifndef tp_maps_GizmoLayer_h
#define tp_maps_GizmoLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/CallbackCollection.h"

namespace tp_maps
{

//##################################################################################################
enum class RotationRingStyle
{
  Compass,
  ArrowsCW,
  ArrowsCCW
};

//##################################################################################################
enum class GizmoScaleMode
{
  World,
  Object,
  Screen,
  ScreenPX
};

//##################################################################################################
class TP_MAPS_EXPORT GizmoLayer: public Layer
{
  friend class HandleDetails;
  TP_DQ;
public:
  //################################################################################################
  GizmoLayer();

  //################################################################################################
  ~GizmoLayer() override;

  //################################################################################################
  bool inInteraction() const;

  //################################################################################################
  void setEnableRotation(bool x, bool y, bool z);

  //################################################################################################
  void setEnableRotationScreen(bool screen);

  //################################################################################################
  void setEnableTranslation(bool x, bool y, bool z);

  //################################################################################################
  void setEnableScale(bool x, bool y, bool z);

  //################################################################################################
  void setRotationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setTranslationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setScaleColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setSelectedColor(const glm::vec3& selectedColor);

  //################################################################################################
  void setScale(const glm::vec3& scale);

  //################################################################################################
  //! This is the radius from the center of the gizmo that the scale arrows start.
  void setCoreSize(const glm::vec3& coreSize);

  //################################################################################################
  //! The thickness of rings
  void setRingHeight(float ringHeight=0.01f);

  //################################################################################################
  void setRingRadius(float outerRadius=1.00f,
                     float innerRadius=0.95f,
                     float spikeRadius=0.90f,
                     float arrowInnerRadius=0.90f,
                     float arrowOuterRadius=1.05f);

  //################################################################################################
  void setRotationRingStyle(RotationRingStyle rotationRingStyle);

  //################################################################################################
  void setGizmoScaleMode(GizmoScaleMode gizmoScaleMode);

  //################################################################################################
  void setGizmoScale(float gizmoScale);

  //################################################################################################
  void setOnlyRenderSelectedAxis(bool onlyRenderSelectedAxis);

  //################################################################################################
  tp_utils::CallbackCollection<void()> changed;

  //################################################################################################
  void setDefaultRenderPass(const RenderPass& defaultRenderPass) override;

protected:

  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;
};

}

#endif
