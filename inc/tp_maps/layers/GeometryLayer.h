#ifndef tp_maps_GeometryLayer_h
#define tp_maps_GeometryLayer_h

#include "tp_maps/Layer.h"

#include "tp_math_utils/Geometry3D.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GeometryLayer: public Layer
{
public:
  //################################################################################################
  GeometryLayer();

  //################################################################################################
  ~GeometryLayer() override;

  //################################################################################################
  const std::vector<tp_math_utils::Geometry>& geometry() const;

  //################################################################################################
  void setGeometry(const std::vector<tp_math_utils::Geometry>& geometry);

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
