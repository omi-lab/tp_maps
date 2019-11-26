#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include <cstring>

namespace tp_maps
{

//##################################################################################################
TextureData TextureData::clone()const
{
  TextureData clone;
  clone.w = w;
  clone.h = h;
  size_t size = w*h;
  if(size>0)
  {
    auto newData = new TPPixel[size];
    clone.data = newData;
    memcpy(newData, data, size*sizeof(TPPixel));
  }
  return clone;
}

//##################################################################################################
TextureData TextureData::clone2()const
{
  auto po2 = [](size_t v)
  {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
  };

  TextureData clone;

  if(w<1||h<1)
  {
    clone.w = w;
    clone.h = h;
    return clone;
  }

  clone.w = tpMax(po2(w), po2(h));
  clone.h = clone.w;

  clone.fw = float(w) / float(clone.w);
  clone.fh = float(h) / float(clone.h);

  size_t size = clone.w*clone.h;

  auto newData = new TPPixel[size];
  clone.data = newData;
  if(clone.w==w && clone.h==h)
  {
    memcpy(newData, data, size*sizeof(TPPixel));
  }
  else
  {
    //size_t padX = (clone.w - w);//*sizeof(TPPixel);
    size_t srcW = w*sizeof(TPPixel);
    for(size_t y=0; y<h; y++)
    {
      TPPixel* dst = newData+(y*clone.w);
      memcpy(dst, data+(y*w), srcW);

      {
        TPPixel* d=dst+w;
        TPPixel  p = *(d-1);
        TPPixel* dMax=dst+clone.w;
        for(; d<dMax; d++)
          (*d) = p;
      }
    }

    {
      size_t dstW = clone.w*sizeof(TPPixel);
      const void* src = clone.data+(clone.w*(h-1));
      for(size_t y=h; y<clone.h; y++)
        memcpy(newData+(clone.w*y), src, dstW);
    }
  }

  return clone;
}

//##################################################################################################
void TextureData::destroy()
{
  delete[] data;
  data=nullptr;
}

//##################################################################################################
struct BasicTexture::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::BasicTexture::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  TextureData image;
  bool imageReady{false};
};

//##################################################################################################
BasicTexture::BasicTexture(Map* map, const TextureData& image):
  Texture(map),
  d(new Private())
{
  setImage(image);
}

//##################################################################################################
BasicTexture::~BasicTexture()
{
  d->image.destroy();
  delete d;
}

//##################################################################################################
void BasicTexture::setImage(const TextureData& image, bool quiet)
{
  d->image.destroy();
  d->image = image.clone2();
  d->imageReady = (d->image.data && d->image.w>0 && d->image.h>0);

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
                               minFilterOption());
  return texture;
}

//##################################################################################################
GLuint BasicTexture::bindTexture(const TextureData& img,
                                 TPGLenum target,
                                 TPGLenum format,
                                 GLint magFilterOption,
                                 GLint minFilterOption)
{
  if(!map()->initialized())
  {
    tpWarning() << "Error! Trying to generate a texture on a map that is not initialized.";
    tp_utils::printStackTrace();
    return 0;
  }

  if(img.w<1 || img.h<1 || !img.data)
  {
    tpWarning() << "BasicTexture::bindTexture() called with null image";
    tp_utils::printStackTrace();
    return 0;
  }

  GLuint txId=0;
  glGenTextures(1, &txId);
  glBindTexture(target, txId);

  glTexImage2D(target, 0, format, int(img.w), int(img.h), 0, format, GL_UNSIGNED_BYTE, img.data);

  if((minFilterOption == GL_NEAREST_MIPMAP_NEAREST) || (minFilterOption == GL_LINEAR_MIPMAP_LINEAR))
    glGenerateMipmap(target);

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterOption);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterOption);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  return txId;
}

//##################################################################################################
glm::vec2 BasicTexture::textureDims()const
{
  return {d->image.fw, d->image.fh};
}

//##################################################################################################
glm::vec2 BasicTexture::imageDims()const
{
  return {float(d->image.w)*d->image.fw, float(d->image.h)*d->image.fh};
}

}
