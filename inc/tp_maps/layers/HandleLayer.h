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
class TP_MAPS_EXPORT HandleDetails
{
  TP_NONCOPYABLE(HandleDetails);
  friend class HandleLayer;
  HandleLayer* m_layer;
public:
  glm::vec3 position;
  glm::vec4 color;
  glm::vec2 offset{0.0f, 0.0f}; //! Offset of the sprite relative to the radius of the sprite.
  size_t sprite;

  float radius;

  void* opaque;

  //################################################################################################
  HandleDetails(HandleLayer* layer,
                const glm::vec3& position_,
                const glm::vec4& color_,
                size_t sprite_,
                float radius_=10.0f,
                int index=0);

  //################################################################################################
  virtual ~HandleDetails();

  //################################################################################################
  HandleLayer* layer() const;
};

//##################################################################################################
//! A layer for rendering movable handles.
/*!
This layer allows the user to move handles on the x,y plane. The model matrix can be used to change
the plane that the handles are moved on.
*/
class TP_MAPS_EXPORT HandleLayer: public Layer
{
  friend class HandleDetails;
  TP_DQ;
public:
  //################################################################################################
  /*!
  \param spriteTexture The sprites used to draw the handles, this takes ownership.
  */
  HandleLayer(SpriteTexture* spriteTexture);

  //################################################################################################
  ~HandleLayer() override;  

  //################################################################################################
  void setPlane(const tp_math_utils::Plane& plane);

  //################################################################################################
  const tp_math_utils::Plane& plane() const;

  //################################################################################################
  float zOffset() const;

  //################################################################################################
  void setZOffset(float zOffset);

  //################################################################################################
  void setAddRemove(bool doubleClickToAdd, bool doubleClickToRemove);

  //################################################################################################
  const std::vector<HandleDetails*>& handles() const;

  //################################################################################################
  void clearHandles();

  //################################################################################################
  void updateHandles();

  //################################################################################################
  void setCalculateHandlePositionCallback(const std::function<bool(HandleDetails*, const glm::ivec2&, const glm::mat4&, glm::vec3&)>& calculateHandlePositionCallback);

  //################################################################################################
  tp_utils::CallbackCollection<void(const HandleDetails*)> handleMoved;

  //################################################################################################
  //! A drag is about to start.
  tp_utils::CallbackCollection<void(const HandleDetails*)> dragStart;

  //################################################################################################
  //! A handle is about to be deleted.
  tp_utils::CallbackCollection<void(const HandleDetails*)> handleDeleted;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

  //################################################################################################
  bool mouseEvent(const MouseEvent& event) override;

  //################################################################################################
  void moveHandle(HandleDetails* handle, const glm::vec3& newPosition);
};

}

#endif
