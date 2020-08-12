#ifndef tp_maps_PostSSAOLayer_h
#define tp_maps_PostSSAOLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostSSAOLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("PostSSAOLayer");
public:
  //################################################################################################
  PostSSAOLayer(Map* map, RenderPass customRenderPass);

  //################################################################################################
  ~PostSSAOLayer()override;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
