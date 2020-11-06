#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include <cstring>

namespace tp_maps
{

////##################################################################################################
//TextureData TextureData::clone() const
//{
//  TextureData clone;
//  clone.w = w;
//  clone.h = h;
//  size_t size = w*h;
//  if(size>0)
//  {
//    auto newData = new TPPixel[size];
//    clone.data = newData;
//    memcpy(newData, data, size*sizeof(TPPixel));
//  }
//  return clone;
//}

////##################################################################################################
//TextureData TextureData::clone2() const
//{
//  TextureData clone;
//  clone2IntoOther(clone);
//  return clone;
//}

////##################################################################################################
//void TextureData::clone2IntoOther(TextureData& clone) const
//{
//  auto po2 = [](size_t v)
//  {
//    v--;
//    v |= v >> 1;
//    v |= v >> 2;
//    v |= v >> 4;
//    v |= v >> 8;
//    v |= v >> 16;
//    v++;
//    return v;
//  };

//  if(w<1||h<1)
//  {
//    clone.w = w;
//    clone.h = h;
//    clone.destroy();
//    return;
//  }

//  size_t existingSize=clone.w*clone.h;

//  clone.w = po2(tpMax(w, h));
//  clone.h = clone.w;

//  clone.fw = (float(w)*fw) / float(clone.w);
//  clone.fh = (float(h)*fh) / float(clone.h);

//  size_t size = clone.w*clone.h;

//  if(!clone.data || existingSize!=size)
//  {
//    clone.destroy();
//    clone.data = new TPPixel[size];
//  }

//  if(clone.w==w && clone.h==h)
//  {
//    memcpy(const_cast<TPPixel*>(clone.data), data, size*sizeof(TPPixel));
//  }
//  else
//  {
//    size_t srcW = w*sizeof(TPPixel);
//    for(size_t y=0; y<h; y++)
//    {
//      TPPixel* dst = const_cast<TPPixel*>(clone.data)+(y*clone.w);
//      memcpy(dst, data+(y*w), srcW);

//      {
//        TPPixel* d=dst+w;
//        TPPixel  p = *(d-1);
//        TPPixel* dMax=dst+clone.w;
//        for(; d<dMax; d++)
//          (*d) = p;
//      }
//    }

//    {
//      size_t dstW = clone.w*sizeof(TPPixel);
//      const void* src = clone.data+(clone.w*(h-1));
//      for(size_t y=h; y<clone.h; y++)
//        memcpy(const_cast<TPPixel*>(clone.data)+(clone.w*y), src, dstW);
//    }
//  }
//}

////##################################################################################################
//void TextureData::destroy()
//{
//  delete[] data;
//  data=nullptr;
//}

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
