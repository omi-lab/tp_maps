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
  int size = w*h;
  if(size>0)
  {
    clone.data = new Pixel[size];
    memcpy(clone.data, data, size*sizeof(Pixel));
  }
  return clone;
}

//##################################################################################################
TextureData TextureData::clone2()const
{
  auto po2 = [](uint32_t v)
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

  int size = clone.w*clone.h;

  clone.data = new Pixel[size];
  if(clone.w==w && clone.h==h)
  {
    memcpy(clone.data, data, size*sizeof(Pixel));
  }
  else
  {
    //size_t padX = (clone.w - w);//*sizeof(Pixel);
    size_t srcW = w*sizeof(Pixel);
    for(int y=0; y<h; y++)
    {
      Pixel* dst = clone.data+(y*clone.w);
      memcpy(dst, data+(y*w), srcW);

      {
        Pixel* d=dst+w;
        Pixel  p = *(d-1);
        Pixel* dMax=dst+clone.w;
        for(; d<dMax; d++)
          (*d) = p;
      }
    }

    {
      size_t dstW = clone.w*sizeof(Pixel);
      void* src = clone.data+(clone.w*(h-1));
      for(int y=h; y<clone.h; y++)
        memcpy(clone.data+(clone.w*y), src, dstW);
    }
  }

  return clone;
}

//##################################################################################################
void TextureData::destroy()
{
  delete[] data;
}

//##################################################################################################
struct BasicTexture::Private
{
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
void BasicTexture::setImage(const TextureData& image)
{
  d->image.destroy();
  d->image = image.clone2();
  d->imageReady = (d->image.data && d->image.w>0 && d->image.h>0);
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
                                 GLenum target,
                                 GLint format,
                                 GLuint magFilterOption,
                                 GLuint minFilterOption)
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

  glTexImage2D(target, 0, format, img.w, img.w, 0, format, GL_UNSIGNED_BYTE, img.data);

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
  return {d->image.w*d->image.fw, d->image.h*d->image.fh};
}

}
