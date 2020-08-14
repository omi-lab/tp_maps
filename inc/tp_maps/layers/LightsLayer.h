#ifndef tp_maps_LightsLayer_h
#define tp_maps_LightsLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"
#include "tp_utils/CallbackCollection.h"

namespace tp_maps
{

//##################################################################################################
//! Draws objects at the light positions.
class TP_MAPS_SHARED_EXPORT LightsLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("LightsLayer");
public:
  //################################################################################################
  LightsLayer();

  //################################################################################################
  ~LightsLayer() override;

  //################################################################################################
  tp_utils::CallbackCollection<void()> lightsEdited;

protected:

  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void lightsChanged(LightingModelChanged lightingModelChanged) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
