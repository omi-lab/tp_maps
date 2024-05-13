#ifndef tp_maps_GizmoLayer_h
#define tp_maps_GizmoLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/CallbackCollection.h"

#include "json.hpp"

namespace tp_maps
{

//##################################################################################################
enum class GizmoRingStyle
{
  Compass,
  ArrowsCW,
  ArrowsCCW
};

//##################################################################################################
std::vector<std::string> gizmoRingStyles();

//##################################################################################################
std::string gizmoRingStyleToString(GizmoRingStyle style);

//##################################################################################################
GizmoRingStyle gizmoRingStyleFromString(const std::string& style);

//##################################################################################################
struct GizmoRingParameters
{
  glm::vec3 color{1.0f, 0.0f, 0.0f};
  glm::vec3 selectedColor{1.0f, 1.0f, 0.0f};
  bool enable{true};
  bool useSelectedColor{false};

  GizmoRingStyle style{GizmoRingStyle::Compass};

  //################################################################################################
  [[nodiscard]] static GizmoRingParameters init(const glm::vec3& color, bool enable)
  {
    GizmoRingParameters p;
    p.color = color;
    p.enable = enable;
    return p;
  }

  //################################################################################################
  [[nodiscard]] GizmoRingParameters applyColor(const glm::vec3& color)
  {
    GizmoRingParameters p = *this;
    p.color = color;
    return p;
  }

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
enum class GizmoArrowStyle
{
  None,
  Stem,
  Stemless
};

//##################################################################################################
std::vector<std::string> gizmoArrowStyles();

//##################################################################################################
std::string gizmoArrowStyleToString(GizmoArrowStyle style);

//##################################################################################################
GizmoArrowStyle gizmoArrowStyleFromString(const std::string& style);

//##################################################################################################
struct GizmoArrowParameters
{
  glm::vec3 color{1.0f, 0.0f, 0.0f};
  glm::vec3 selectedColor{1.0f , 1.0f, 0.0f};
  bool enable{true};
  bool useSelectedColor{false};

  float stemStart  = 0.1f;
  float stemLength = 0.7f;
  float stemRadius = 0.05f;
  float coneRadius = 0.1f;
  float coneLength = 0.2f;

  GizmoArrowStyle positiveArrowStyle{GizmoArrowStyle::Stem};
  GizmoArrowStyle negativeArrowStyle{GizmoArrowStyle::None};

  //################################################################################################
  [[nodiscard]] static GizmoArrowParameters init(const glm::vec3& color, bool enable)
  {
    GizmoArrowParameters p;
    p.color = color;
    p.enable = enable;
    return p;
  }

  //################################################################################################
  [[nodiscard]] GizmoArrowParameters static initTranslation(const glm::vec3& color, bool enable)
  {
    GizmoArrowParameters p = init(color, enable);
    p.stemLength = 0.3f;
    return p;
  }

  //################################################################################################
  [[nodiscard]] GizmoArrowParameters static initScale(const glm::vec3& color, bool enable)
  {
    GizmoArrowParameters p = init(color, enable);
    p.stemLength = 0.3f;
    return p;
  }

  //################################################################################################
  [[nodiscard]] GizmoArrowParameters applyColor(const glm::vec3& color)
  {
    GizmoArrowParameters p = *this;
    p.color = color;
    return p;
  }

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
struct GizmoPlaneParameters
{
  glm::vec3 color{1.0f, 0.0f, 0.0f};
  glm::vec3 selectedColor{1.0f, 1.0f, 0.0f};
  bool enable{true};
  bool useSelectedColor{false};

  float size{0.5f};
  float radius{0.3f};
  float padding{0.40f};

  bool center{false};

  //################################################################################################
  [[nodiscard]] static GizmoPlaneParameters init(const glm::vec3& color, bool enable)
  {
    GizmoPlaneParameters p;
    p.color = color;
    p.enable = enable;
    return p;
  }

  //################################################################################################
  [[nodiscard]] static GizmoPlaneParameters initCenter(const glm::vec3& color, bool enable)
  {
    GizmoPlaneParameters p = init(color, enable);
    p.center = true;
    return p;
  }

  //################################################################################################
  [[nodiscard]] GizmoPlaneParameters applyColor(const glm::vec3& color)
  {
    GizmoPlaneParameters p = *this;
    p.color = color;
    return p;
  }

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
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
std::vector<std::string> gizmoScaleModes();

//##################################################################################################
std::string gizmoScaleModeToString(GizmoScaleMode mode);

//##################################################################################################
GizmoScaleMode gizmoScaleModeFromString(const std::string& mode);

//##################################################################################################
enum class GizmoRenderPass
{
  Normal,
  GUI3D
};

//##################################################################################################
std::vector<std::string> gizmoRenderPasses();

//##################################################################################################
std::string gizmoRenderPassToString(GizmoRenderPass renderPass);

//##################################################################################################
GizmoRenderPass gizmoRenderPassFromString(const std::string& renderPass);

//##################################################################################################
struct GizmoParameters
{
  GizmoRenderPass gizmoRenderPass{GizmoRenderPass::GUI3D};
  GizmoRenderPass referenceLinesRenderPass{GizmoRenderPass::Normal};

  GizmoScaleMode gizmoScaleMode{GizmoScaleMode::Object};
  float gizmoScale{1.0f};
  bool onlyRenderSelectedAxis{false};

  GizmoRingParameters rotationX{GizmoRingParameters::init({1.0f, 0.0f, 0.0f}, true)};
  GizmoRingParameters rotationY{GizmoRingParameters::init({0.0f, 1.0f, 0.0f}, true)};
  GizmoRingParameters rotationZ{GizmoRingParameters::init({0.0f, 0.0f, 1.0f}, true)};

  GizmoRingParameters rotationScreen{GizmoRingParameters::init({0.5f, 0.5f, 0.5f}, false)};

  GizmoArrowParameters translationArrowX{GizmoArrowParameters::initTranslation({1.0f, 0.0f, 0.0f}, true)};
  GizmoArrowParameters translationArrowY{GizmoArrowParameters::initTranslation({0.0f, 1.0f, 0.0f}, true)};
  GizmoArrowParameters translationArrowZ{GizmoArrowParameters::initTranslation({0.0f, 0.0f, 1.0f}, true)};

  GizmoPlaneParameters translationPlaneX{GizmoPlaneParameters::init({1.0f, 0.0f, 0.0f}, false)};
  GizmoPlaneParameters translationPlaneY{GizmoPlaneParameters::init({0.0f, 1.0f, 0.0f}, false)};
  GizmoPlaneParameters translationPlaneZ{GizmoPlaneParameters::init({0.0f, 0.0f, 1.0f}, false)};

  GizmoPlaneParameters translationPlaneScreen{GizmoPlaneParameters::initCenter({0.5f, 0.5f, 0.5f}, false)};

  GizmoArrowParameters scaleArrowX{GizmoArrowParameters::initScale({1.0f, 0.0f, 0.0f}, true)};
  GizmoArrowParameters scaleArrowY{GizmoArrowParameters::initScale({0.0f, 1.0f, 0.0f}, true)};
  GizmoArrowParameters scaleArrowZ{GizmoArrowParameters::initScale({0.0f, 0.0f, 1.0f}, true)};

  //################################################################################################
  void disableRotation()
  {
    rotationX.enable = false;
    rotationY.enable = false;
    rotationZ.enable = false;

    rotationScreen.enable = false;
  }

  //################################################################################################
  void disableTranslation()
  {
    translationArrowX.enable = false;
    translationArrowY.enable = false;
    translationArrowZ.enable = false;

    translationPlaneX.enable = false;
    translationPlaneY.enable = false;
    translationPlaneZ.enable = false;

    translationPlaneScreen.enable = false;
  }

  //################################################################################################
  void disableScale()
  {
    scaleArrowX.enable = false;
    scaleArrowY.enable = false;
    scaleArrowZ.enable = false;
  }

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
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
  void setParameters(const GizmoParameters& params);

  //################################################################################################
  const GizmoParameters& parameters() const;

  //################################################################################################
  void setEnableRotation(bool x, bool y, bool z);

  //################################################################################################
  void setEnableRotationScreen(bool screen);

  //################################################################################################
  void setEnableTranslationArrows(bool x, bool y, bool z);

  //################################################################################################
  void setEnableTranslationPlanes(bool x, bool y, bool z);

  //################################################################################################
  void setEnableTranslationPlanesScreenScreen(bool screen);

  //################################################################################################
  void setEnableScale(bool x, bool y, bool z);

  //################################################################################################
  void setRotationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setRotationScreenColor(const glm::vec3& color);

  //################################################################################################
  void setTranslationArrowColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setTranslationPlaneColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z);

  //################################################################################################
  void setPlaneTranslationScreenColor(const glm::vec3& color);

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
  void setGizmoRingStyle(GizmoRingStyle gizmoRingStyle);

  //################################################################################################
  void setGizmoScaleMode(GizmoScaleMode gizmoScaleMode);

  //################################################################################################
  void setGizmoScale(float gizmoScale);

  //################################################################################################
  void setTranslationArrowParameters(const GizmoArrowParameters& x,
                                     const GizmoArrowParameters& y,
                                     const GizmoArrowParameters& z);

  //################################################################################################
  void setScaleArrowParameters(const GizmoArrowParameters& x,
                               const GizmoArrowParameters& y,
                               const GizmoArrowParameters& z);

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
