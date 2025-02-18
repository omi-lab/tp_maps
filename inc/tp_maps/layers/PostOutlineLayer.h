#ifndef tp_maps_PostOutlineLayer_h
#define tp_maps_PostOutlineLayer_h

#include "tp_maps/layers/PostLayer.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostOutlineLayer: public PostLayer
{
  TP_DQ;
public:
  //################################################################################################
  PostOutlineLayer();

  //################################################################################################
  ~PostOutlineLayer();

protected:
  //################################################################################################
  PostShader* makeShader() override;

  //################################################################################################
  void addRenderPasses(std::vector<RenderPass>& renderPasses) override;

  //################################################################################################
  void render(tp_maps::RenderInfo& renderInfo) override;
};

}

#endif
