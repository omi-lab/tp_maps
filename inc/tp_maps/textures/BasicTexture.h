#ifndef tp_maps_BasicTexture_h
#define tp_maps_BasicTexture_h

#include "tp_maps/Texture.h"

#include "tp_image_utils/ColorMap.h"

#include "tp_utils/TPPixel.h"
#include "tp_utils/RefCount.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_SHARED_EXPORT BasicTexture : public Texture
{
  TP_REF_COUNT_OBJECTS("BasicTexture");
public:
  //################################################################################################
  BasicTexture(Map* map, const tp_image_utils::ColorMap& image=tp_image_utils::ColorMap(), bool makeSquare=true);

  //################################################################################################
  ~BasicTexture() override;

  //################################################################################################
  void setImage(const tp_image_utils::ColorMap& image, bool quiet=false);

  //################################################################################################
  bool imageReady() override;

  //################################################################################################
  GLuint bindTexture() override;

  //################################################################################################
  //! Creates and binds a texure with the given image
  /*!
  \param img: The bitmap image for the texture
  \param target: The target type (normally GL_TEXTURE_2D)
  \param format: The format (normally GL_RGBA)
  \param magFilterOption: The texture magnification function to use
  \param minFilterOption: The texture minifying function to use
  \return the id for the new texture
  */
  GLuint bindTexture(const tp_image_utils::ColorMap& img,
                     TPGLenum target,
                     TPGLenum format,
                     GLint magFilterOption,
                     GLint minFilterOption,
                     GLint textureWrapS = GL_CLAMP_TO_EDGE,
                     GLint textureWrapT = GL_CLAMP_TO_EDGE);

  //################################################################################################
  glm::vec2 textureDims() const override;

  //################################################################################################
  glm::vec2 imageDims() const override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
