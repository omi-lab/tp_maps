#ifndef tp_maps_GizmoLayer_h
#define tp_maps_GizmoLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/CallbackCollection.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GizmoLayer: public Layer
{
  friend class HandleDetails;
public:
  //################################################################################################
  GizmoLayer();

  //################################################################################################
  ~GizmoLayer()override;

  //################################################################################################
  void setEnableRotation(bool x, bool y, bool z);

  //################################################################################################
  void setScale(const glm::vec3& scale);

  //################################################################################################
  tp_utils::CallbackCollection<void()> changed;


protected:

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
