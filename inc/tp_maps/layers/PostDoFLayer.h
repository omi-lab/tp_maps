#ifndef tp_maps_PostDoFLayer_h
#define tp_maps_PostDoFLayer_h

#include "tp_maps/layers/PostLayer.h"

#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace omi_scene_3d
{
struct DoFParameters;
}
namespace os3 = omi_scene_3d;

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT PostDoFLayer: public PostLayer
{
public:
  //################################################################################################
  PostDoFLayer();

  //################################################################################################
  ~PostDoFLayer();

  //################################################################################################
  const PostDoFParameters& parameters() const;

  //################################################################################################
  void setParameters(const PostDoFParameters& parameters);

  //################################################################################################
  float calculateFStopDistance(float fStop) const;

  //################################################################################################
  void setDofFromScene(const os3::DoFParameters& dofParams);

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
