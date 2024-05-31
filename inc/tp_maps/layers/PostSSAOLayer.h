#ifndef tp_maps_PostSSAOLayer_h
#define tp_maps_PostSSAOLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/PostSSAOShader.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostSSAOLayer: public PostLayer
{
  TP_DQ;
public:
  //################################################################################################
  PostSSAOLayer();

  //################################################################################################
  ~PostSSAOLayer() override;

  //################################################################################################
  const PostSSAOParameters& parameters() const;

  //################################################################################################
  void setParameters(const PostSSAOParameters& parameters);

protected:
  //################################################################################################
  PostShader* makeShader() override;

  //################################################################################################
  void addRenderPasses(std::vector<RenderPass>& renderPasses) override;
};

}

#endif
