#ifndef tp_maps_PostDoFLayer_h
#define tp_maps_PostDoFLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"

namespace omi_scene_3d
{
struct DoFParameters;
}

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostDoFLayer: public PostLayer
{
public:
  //################################################################################################
  PostDoFLayer();

  //################################################################################################
  ~PostDoFLayer();

  //################################################################################################
  const DepthOfFieldShaderParameters& parameters() const;

  //################################################################################################
  void setParameters(const DepthOfFieldShaderParameters& parameters);

  //################################################################################################
  float calculateFStopDistance(float fStop) const;

  //################################################################################################
  void setDofFromScene( const omi_scene_3d::DoFParameters& dofParams );

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
