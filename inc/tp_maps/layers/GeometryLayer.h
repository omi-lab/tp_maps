#ifndef tp_maps_GeometryLayer_h
#define tp_maps_GeometryLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/shaders/MaterialShader.h"

namespace tp_maps
{

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Geometry
{
  std::vector<glm::vec3> geometry;
  MaterialShader::Material material;
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GeometryLayer: public Layer
{
public:
  //################################################################################################
  GeometryLayer();

  //################################################################################################
  ~GeometryLayer()override;

  //################################################################################################
  const std::vector<Geometry>& geometry()const;

  //################################################################################################
  void setGeometry(const std::vector<Geometry>& geometry);

  //################################################################################################
  //! Call this to set the lighting
  void setLight(const MaterialShader::Light& light);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
