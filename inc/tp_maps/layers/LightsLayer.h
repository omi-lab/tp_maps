#ifndef tp_maps_LightsLayer_h
#define tp_maps_LightsLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"
#include "tp_utils/CallbackCollection.h"

namespace tp_maps
{
class FontRenderer;
//##################################################################################################
//! Draws objects at the light positions.
class TP_MAPS_EXPORT LightsLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("LightsLayer");
public:
  //################################################################################################
  LightsLayer(bool editLights);

  //################################################################################################
  ~LightsLayer() override;

  //################################################################################################
  bool editLights() const;

  //################################################################################################
  void setEditLights(bool editLights);

  //################################################################################################
  //! Set the font that will be used to render text
  /*!
  \note This does not take ownership.
  \param font The font to use for drawing the labels for the lights.
  */
  void setFont(FontRenderer* font);

  //################################################################################################
  FontRenderer* font() const;

  //################################################################################################
  const std::string& prefix() const;

  //################################################################################################
  void setPrefix(const std::string& prefix);

  //################################################################################################
  void setLightIndex(size_t lightIndex);

  //################################################################################################
  size_t lightIndex() const;

  tp_utils::CallbackCollection<void()> lightIndexChanged;

  //################################################################################################
  tp_utils::CallbackCollection<void()> lightsEdited;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  bool mouseEvent(const tp_maps::MouseEvent& event) override;

  //################################################################################################
  void lightsChanged(LightingModelChanged lightingModelChanged) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
