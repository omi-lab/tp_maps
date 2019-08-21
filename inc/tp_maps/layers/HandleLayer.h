#ifndef tp_maps_HandleLayer_h
#define tp_maps_HandleLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/SpriteTexture.h"

#include "glm/glm.hpp"

#include <functional>

namespace tp_math_utils
{
class Plane;
}

namespace tp_maps
{
class HandleLayer;
class Texture;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT HandleDetails
{
  friend class HandleLayer;
  HandleLayer* m_layer;
public:
  glm::vec3 position;
  glm::vec4 color;
  int sprite;

  float radius;

  void* opaque;
  void(*movedCallback)(HandleDetails* handle, const glm::vec3& newPosition);

  //################################################################################################
  HandleDetails(HandleLayer* layer,
                const glm::vec3& position_,
                const glm::vec4& color_,
                int sprite,
                float radius_=10.0f,
                int index=0);

  //################################################################################################
  virtual ~HandleDetails();

  //################################################################################################
  HandleLayer* layer()const;
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT HandleLayer: public Layer
{
  friend class HandleDetails;
public:
  //################################################################################################
  HandleLayer(SpriteTexture* spriteTexture);

  //################################################################################################
  ~HandleLayer()override;  

  //################################################################################################
  const std::vector<HandleDetails*>& handles()const;

  //################################################################################################
  void clearHandles();

  //################################################################################################
  //! Returns the plane that handles move on
  const tp_math_utils::Plane& plane()const;

  //################################################################################################
  //! Sets the plane that handles move on
  void setPlane(const tp_math_utils::Plane& plane);

  //################################################################################################
  void setHandleMovedCallback(const std::function<void(void)>& handleMovedCallback);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  void moveHandle(HandleDetails* handle, const glm::vec3& newPosition);

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
