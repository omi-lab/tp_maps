#ifndef tp_maps_ImageLayer_h
#define tp_maps_ImageLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
class Texture;
class ImageShader;

//##################################################################################################
class TP_MAPS_EXPORT ImageLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("ImageLayer");
public:
  //################################################################################################
  /*!
  \param texture The image in OpenGL coordinate system (0,0) in bottom left. This takes ownership.
  */
  ImageLayer(Texture* texture);

  //################################################################################################
  ~ImageLayer() override;

  //################################################################################################
  //! Sets the color that is used to modify the image
  void setColor(const glm::vec4& color);

  //################################################################################################
  glm::vec4 color() const;

  //################################################################################################
  void setImageCoords(const glm::vec3& topRight,
                      const glm::vec3& bottomRight,
                      const glm::vec3& bottomLeft,
                      const glm::vec3& topLeft);

  //################################################################################################
  void setShader(const std::function<ImageShader*(Map*)>& getShader);

  //################################################################################################
  //! Bind the texture in the next render pass without triggering an update.
  void bindTextureInNextRender();

protected:
  //################################################################################################
  virtual glm::mat4 calculateMatrix() const;

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
