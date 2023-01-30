#ifndef tp_maps_PostSSAOLayer_h
#define tp_maps_PostSSAOLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/PostSSAOShader.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostSSAOLayer: public PostLayer
{
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

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
