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
  const glm::mat4& objectMatrix()const;

  //################################################################################################
  void setObjectMatrix(const glm::mat4& objectMatrix);

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
