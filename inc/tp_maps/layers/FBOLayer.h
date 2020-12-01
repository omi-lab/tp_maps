#ifndef tp_maps_FBOLayer_h
#define tp_maps_FBOLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
enum class FBOLayerSource
{
  CurrentReadColor,
  CurrentReadDepth,
  CurrentDrawColor,
  CurrentDrawDepth,
  LightColor,
  LightDepth
};

//##################################################################################################
//! Display the contens of an FBO on screen.
/*!
Various FBOs are used internally to generate various effects including reflection and shadows. This
Layer allows you to present the contents of these FBOs on screen to aid in debugging.
*/
class TP_MAPS_SHARED_EXPORT FBOLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("FBOLayer");
public:
  //################################################################################################
  FBOLayer(FBOLayerSource source=FBOLayerSource::CurrentReadColor,
           size_t index=0,
           const glm::vec2& origin={0.75f, 0.75f},
           const glm::vec2& size={0.20f, 0.20f});

  //################################################################################################
  ~FBOLayer() override;

  //################################################################################################
  //! Set the geometry of the image, values are as a fraction of the screen so in the range 0 to 1.
  void setImageCoords(const glm::vec2& origin, const glm::vec2& size);

  //################################################################################################
  void setSource(FBOLayerSource source=FBOLayerSource::CurrentReadColor, size_t index=0);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
