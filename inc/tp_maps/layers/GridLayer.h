#ifndef tp_maps_GridLayer_h
#define tp_maps_GridLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{

//################################################################################################
class TP_MAPS_SHARED_EXPORT GridLayer: public Layer
{
public:
  //################################################################################################
  GridLayer();

  //################################################################################################
  ~GridLayer() override;

protected:
  //################################################################################################
  virtual void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
