#ifndef tp_maps_PostLayer_h
#define tp_maps_PostLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{

class PostShader;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PostLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("PostLayer");
public:
  //################################################################################################
  PostLayer(Map* map, RenderPass customRenderPass);

  //################################################################################################
  ~PostLayer() override;

  //################################################################################################
  void setFrameCoordinateSystem(const tp_utils::StringID& frameCoordinateSystem);

  //################################################################################################
  //! If true just blit read to draw buffers.
  bool bypass() const;

  //################################################################################################
  void setBypass(bool bypass);

  //################################################################################################
  //! Make the post layer a rectangle, size=1=fullscreen
  void setRectangle(const glm::vec2& size);

  //################################################################################################
  //! Make the post layer a rectangle, size=1=fullscreen
  /*!
  Typically this is used where the coordinateSystem() chosen for this layer view a larger area of
  the scene and you want to place a frame around it. Think of the camera view in blender where you
  have the camera in the middle surrounded by a semi transparent frame. This mode is used to draw
  frames like that.

  \param holeSize at the middle of the frame, 1=fullscreen
  \param size at the outside of the frame, 1=fullscreen
   */
  void setFrame(const glm::vec2& holeSize, const glm::vec2& size);  

  //################################################################################################
  void setBlit(bool blitRectangle, bool blitFrame);


protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

  //################################################################################################
  virtual PostShader* makeShader()=0;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
