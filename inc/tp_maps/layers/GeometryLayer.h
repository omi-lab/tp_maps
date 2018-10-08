#ifndef tp_maps_GeometryLayer_h
#define tp_maps_GeometryLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/shaders/MaterialShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{
class GeometryLayer;
class Texture;

//##################################################################################################
struct Geometry
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
