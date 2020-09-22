#ifndef tp_maps_BasicTexture_h
#define tp_maps_BasicTexture_h

#include "tp_maps/Texture.h"

#include "tp_image_utils/ColorMap.h"

#include "tp_utils/TPPixel.h"
#include "tp_utils/RefCount.h"

namespace tp_maps
{

////##################################################################################################
//struct TP_MAPS_SHARED_EXPORT TextureData
//{
//  // Note that 0,0 is in the bottom left.
//  size_t w{0};
//  size_t h{0};

//  //Used for textures that have been padded to make them a power of 2.
//  //These will be a value between 0.5f and 1.0f.
//  float fw{1.0f};
//  float fh{1.0f};
//  const TPPixel* data{nullptr};

//  //################################################################################################
//  TextureData clone() const;

//  //################################################################################################
//  //! Clone the texture and pad to a power of 2
//  TextureData clone2() const;

//  //################################################################################################
//  //! Clone the texture and pad to a power of 2 into an existing texture
//  void clone2IntoOther(TextureData& clone) const;

//  //################################################################################################
//  void destroy();
//};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT BasicTexture : public Texture
{
  TP_REF_COUNT_OBJECTS("BasicTexture");
public:
  //################################################################################################
  BasicTexture(Map* map, const tp_image_utils::ColorMap& image=tp_image_utils::ColorMap());

  //################################################################################################
  ~BasicTexture() override;

  //################################################################################################
  void setImage(const tp_image_utils::ColorMap& image, bool quiet=false);

  //################################################################################################
  bool imageReady()override;

  //################################################################################################
  GLuint bindTexture()override;

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
