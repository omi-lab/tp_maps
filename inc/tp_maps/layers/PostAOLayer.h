#ifndef tp_maps_AmbientOcclusionLayer_h
#define tp_maps_AmbientOcclusionLayer_h

#include "tp_maps/layers/PostLayer.h"
#include "tp_maps/shaders/PostAOBaseShader.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostAOLayer: public PostLayer
{
public:
  //################################################################################################
  PostAOLayer();

  //################################################################################################
  ~PostAOLayer() override;

  //################################################################################################
  const PostAOParameters& parameters() const;

  //################################################################################################
  void setParameters(const PostAOParameters& parameters);

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
