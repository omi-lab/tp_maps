#ifndef tp_maps_AmbientOcclusionLayer_h
#define tp_maps_AmbientOcclusionLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/AmbientOcclusionParameters.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT AmbientOcclusionLayer: public PostLayer
{
public:
  //################################################################################################
  AmbientOcclusionLayer();

  //################################################################################################
  ~AmbientOcclusionLayer() override;

  //################################################################################################
  const AmbientOcclusionParameters& parameters() const;

  //################################################################################################
  void setParameters(const AmbientOcclusionParameters& parameters);

protected:
  //################################################################################################
  PostShader* makeShader() override;

  //################################################################################################
  void addRenderPasses(std::vector<RenderPass>& renderPasses) override;

  //################################################################################################
  void render(tp_maps::RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
