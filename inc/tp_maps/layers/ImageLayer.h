#ifndef tp_maps_ImageLayer_h
#define tp_maps_ImageLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class Texture;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT ImageLayer: public Layer
{
public:
  //################################################################################################
  /*!
  \param texture The image in OpenGL coordinate system (0,0) in bottom left.
  */
  ImageLayer(Texture* texture);

  //################################################################################################
  ~ImageLayer()override;

  //################################################################################################
  //! Sets the color that is used to modify the image
  void setColor(const glm::vec4& color);

  //################################################################################################
  glm::vec4 color()const;

  //################################################################################################
  void setImageCoords(const glm::vec3& topRight,
                      const glm::vec3& bottomRight,
                      const glm::vec3& bottomLeft,
                      const glm::vec3& topLeft);

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
