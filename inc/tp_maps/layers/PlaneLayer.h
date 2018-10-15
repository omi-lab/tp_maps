#ifndef tp_maps_PlaneLayer_h
#define tp_maps_PlaneLayer_h

#include "tp_maps/layers/LinesLayer.h"
#include "tp_maps/shaders/MaterialShader.h"

#include "glm/glm.hpp"

namespace tp_math_utils
{
class Plane;
}

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PlaneLayer: public LinesLayer
{
public:
  //################################################################################################
  PlaneLayer();

  //################################################################################################
  ~PlaneLayer()override;

  //################################################################################################
  const tp_math_utils::Plane& plane()const;

  //################################################################################################
  void setPlane(const tp_math_utils::Plane& plane);

  //################################################################################################
  const glm::vec4& color()const;

  //################################################################################################
  void setColor(const glm::vec4& color);

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
