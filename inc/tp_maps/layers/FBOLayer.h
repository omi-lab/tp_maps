#ifndef tp_maps_FBOLayer_h
#define tp_maps_FBOLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

#include "json.hpp" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
enum class FBOLayerSource
{
  Color,
  Depth,
  Normals,
  Specular,
  Picking
};

//##################################################################################################
std::string fboLayerSourceToString(FBOLayerSource fboLayerSource);

//##################################################################################################
FBOLayerSource fboLayerSourceFromString(const std::string& fboLayerSource);

//##################################################################################################
std::vector<std::string> fboLayerSources();

//##################################################################################################
struct FBOWindow
{
  tp_utils::StringID fboName;                        //!< The name of the FBO to display.
  FBOLayerSource     source {FBOLayerSource::Color}; //!< The texture to display.
  glm::vec2          origin {0.75f, 0.75f};          //!< The origin of the window.
  glm::vec2          size   {0.20f, 0.20f};          //!< The size of the window.

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
//! Display the contens of an FBO on screen.
/*!
Various FBOs are used internally to generate various effects including reflection and shadows. This
Layer allows you to present the contents of these FBOs on screen to aid in debugging.
*/
class TP_MAPS_EXPORT FBOLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("FBOLayer");
  TP_DQ;
public:
  //################################################################################################
  FBOLayer();

  //################################################################################################
  ~FBOLayer() override;

  //################################################################################################
  void setWindows(const std::vector<FBOWindow>& windows);

  //################################################################################################
  const std::vector<FBOWindow>& windows() const;

  //################################################################################################
  void setPollPicking(bool pollPicking);

  //################################################################################################
  bool pollPicking() const;

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

  //################################################################################################
  void animate(double timestampMS) override;
};

}

#endif
