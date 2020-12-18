#ifndef tp_maps_PostLayer_h
#define tp_maps_PostLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{

class PostShader;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("PostLayer");
public:
  //################################################################################################
  PostLayer(Map* map, RenderPass customRenderPass);

  //################################################################################################
  ~PostLayer() override;

  //################################################################################################
  //! If true just blit read to draw buffers.
  bool bypass() const;

  //################################################################################################
  void setBypass(bool bypass);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  virtual PostShader* makeShader()=0;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
