#ifndef tp_maps_DepthOfFieldBlurLayer_h
#define tp_maps_DepthOfFieldBlurLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"

namespace omi_scene_3d
{
struct DoFParameters;
}

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT DepthOfFieldBlurLayer: public PostLayer
{
public:
  //################################################################################################
  DepthOfFieldBlurLayer(Map* map,
                        tp_maps::RenderPass customRenderPass1,
                        tp_maps::RenderPass customRenderPass2,
                        tp_maps::RenderPass customRenderPass3,
                        tp_maps::RenderPass customRenderPass4,
                        tp_maps::RenderPass customRenderPass5,
                        tp_maps::RenderPass customRenderPass6);

  //################################################################################################
  ~DepthOfFieldBlurLayer();

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
