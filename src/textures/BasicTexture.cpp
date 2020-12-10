#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include <cstring>

namespace tp_maps
{

//##################################################################################################
struct BasicTexture::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::BasicTexture::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  tp_image_utils::ColorMap image;
  bool imageReady{false};
};

//##################################################################################################
BasicTexture::BasicTexture(Map* map, const tp_image_utils::ColorMap& image):
  Texture(map),
  d(new Private())
{
  setImage(image);
}

//##################################################################################################
BasicTexture::~BasicTexture()
{
  delete d;
}

//##################################################################################################
void BasicTexture::setImage(const tp_image_utils::ColorMap& image, bool quiet)
{
  image.clone2IntoOther(d->image);
  d->imageReady = (d->image.constData() && d->image.width()>0 && d->image.height()>0);

  if(!quiet)
    imageChanged();
}

//##################################################################################################
bool BasicTexture::imageReady()
{
  return d->imageReady;
}

//##################################################################################################
GLuint BasicTexture::bindTexture()
{
  if(!d->imageReady)
    return 0;

  GLuint texture = bindTexture(d->image,
                               GL_TEXTURE_2D,
                               GL_RGBA,
                               magFilterOption(),
                               minFilterOption(),
                               textureWrapS(),
                               textureWrapT());

  return texture;
}

//##################################################################################################
GLuint BasicTexture::bindTexture(const tp_image_utils::ColorMap& img,
                                 TPGLenum target,
                                 TPGLenum format,
                                 GLint magFilterOption,
                                 GLint minFilterOption,
                                 GLint textureWrapS,
                                 GLint textureWrapT)
{
  if(!map()->initialized())
  {
    tpWarning() << "Error! Trying to generate a texture on a map that is not initialized.";
    tp_utils::printStackTrace();
    return 0;
  }

  if(img.width()<1 || img.height()<1 || !img.constData())
  {
    tpWarning() << "BasicTexture::bindTexture() called with null image";
    tp_utils::printStackTrace();
    return 0;
  }

  GLuint txId=0;
  glGenTextures(1, &txId);
  glBindTexture(target, txId);

  glTexImage2D(target, 0, format, int(img.width()), int(img.height()), 0, format, GL_UNSIGNED_BYTE, img.constData());

  if((minFilterOption == GL_NEAREST_MIPMAP_NEAREST) || (minFilterOption == GL_LINEAR_MIPMAP_LINEAR))
    glGenerateMipmap(target);

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterOption);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterOption);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT);

#ifdef TP_LINUX
  {
    float maxAnisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &maxAnisotropy);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
  }
#endif

  return txId;
}

//##################################################################################################
glm::vec2 BasicTexture::textureDims() const
{
  return {d->image.fw(), d->image.fh()};
}

//##################################################################################################
glm::vec2 BasicTexture::imageDims() const
{
  return {float(d->image.width())*d->image.fw(), float(d->image.height())*d->image.fh()};
}

}
