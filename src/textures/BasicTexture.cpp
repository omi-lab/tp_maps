#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/TimeUtils.h"

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
  NChannels nChannels{NChannels::RGBA};
  bool imageReady{false};
  bool makeSquare{true};
};

//##################################################################################################
BasicTexture::BasicTexture(Map* map,
                           const tp_image_utils::ColorMap& image,
                           NChannels nChannels,
                           bool makeSquare):
  Texture(map),
  d(new Private())
{
  d->makeSquare = makeSquare;
  setImage(image, nChannels);
}

//##################################################################################################
BasicTexture::~BasicTexture()
{
  delete d;
}

//##################################################################################################
void BasicTexture::setImage(const tp_image_utils::ColorMap& image,
                            NChannels nChannels,
                            bool quiet)
{
  if(d->makeSquare)
    image.clone2IntoOther(d->image);
  else
    d->image = image;

  d->nChannels = nChannels;

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
void BasicTexture::updateContent(GLuint texId)
{
  if(!d->imageReady)
    return;

  glBindTexture(GL_TEXTURE_2D, texId);

  TPGLenum format = d->nChannels==NChannels::RGB?GL_RGB:GL_RGBA;

  switch(map()->shaderProfile())
  {
  case ShaderProfile::GLSL_110: [[fallthrough]];
  case ShaderProfile::GLSL_120: [[fallthrough]];
  case ShaderProfile::GLSL_130: [[fallthrough]];
  case ShaderProfile::GLSL_140: [[fallthrough]];
  case ShaderProfile::GLSL_150: [[fallthrough]];
  case ShaderProfile::GLSL_330: [[fallthrough]];
  case ShaderProfile::GLSL_400: [[fallthrough]];
  case ShaderProfile::GLSL_410: [[fallthrough]];
  case ShaderProfile::GLSL_420: [[fallthrough]];
  case ShaderProfile::GLSL_430: [[fallthrough]];
  case ShaderProfile::GLSL_440: [[fallthrough]];
  case ShaderProfile::GLSL_450: [[fallthrough]];
  case ShaderProfile::GLSL_460:
  {
    glTexSubImage2D(GL_TEXTURE_2D, 0 /*level */, 0 /* x offset */, 0 /* y offset */, int(d->image.width()), int(d->image.height()), GL_RGBA, GL_UNSIGNED_BYTE, d->image.constData());
    break;
  }

  case ShaderProfile::GLSL_100_ES: [[fallthrough]];
  case ShaderProfile::GLSL_300_ES: [[fallthrough]];
  case ShaderProfile::GLSL_310_ES: [[fallthrough]];
  case ShaderProfile::GLSL_320_ES:
  {
    if(format == GL_RGB)
    {
      // For GL ES we seem to need the internalFormat and format to be the same, so here we take the
      // RGBA data and pack it as RGB.
      struct RGB
      {
        uint8_t r;
        uint8_t g;
        uint8_t b;
      };
      std::vector<RGB> packed;
      packed.resize(d->image.size());

      auto dst = packed.begin();
      auto src = d->image.constData();
      for(; dst!=packed.end(); src++, ++dst)
      {
        dst->r = src->r;
        dst->g = src->g;
        dst->b = src->b;
      }

      // Each pixel is 3 bytes so row alignment may not be 4 bytes so set it to 1
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexSubImage2D(GL_TEXTURE_2D, 0 /*level */, 0 /* x offset */, 0 /* y offset */, int(d->image.width()), int(d->image.height()), GL_RGB, GL_UNSIGNED_BYTE, packed.data());
      glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    else if(format == GL_RGBA)
    {
      glTexSubImage2D(GL_TEXTURE_2D, 0 /*level */, 0 /* x offset */, 0 /* y offset */, int(d->image.width()), int(d->image.height()), GL_RGBA, GL_UNSIGNED_BYTE, d->image.constData());
    }

    break;
  }

  case ShaderProfile::HLSL_10:
  case ShaderProfile::HLSL_11:
  case ShaderProfile::HLSL_12:
  case ShaderProfile::HLSL_13:
  case ShaderProfile::HLSL_14:
  case ShaderProfile::HLSL_20:
  case ShaderProfile::HLSL_20a:
  case ShaderProfile::HLSL_20b:
  case ShaderProfile::HLSL_30:
  case ShaderProfile::HLSL_40:
  case ShaderProfile::HLSL_41:
  case ShaderProfile::HLSL_50:
  case ShaderProfile::HLSL_51:
  case ShaderProfile::HLSL_60:
  case ShaderProfile::HLSL_61:
  case ShaderProfile::HLSL_62:
  case ShaderProfile::HLSL_63:
  case ShaderProfile::HLSL_64:
  case ShaderProfile::HLSL_65:
  case ShaderProfile::HLSL_66:
  case ShaderProfile::HLSL_67:
  {
    tpWarning() << "HLSL not implemented.";
    break;
  }
  }

}

//##################################################################################################
GLuint BasicTexture::bindTexture()
{
  if(!d->imageReady)
    return 0;

  GLuint texture = bindTexture(d->image,
                               GL_TEXTURE_2D,
                               d->nChannels==NChannels::RGB?GL_RGB:GL_RGBA,
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
  TP_FUNCTION_TIME("BasicTexture::bindTexture");

#if 0
  tp_utils::ElapsedTimer t;
  t.start();
  TP_CLEANUP([&]
  {
    if(t.elapsed()>5)
    {
      static size_t fileIndex{0};
      tp_image_utils::saveImage("C:/Users/PC/Desktop/basic_texture/" + tp_utils::fixedWidthKeepRight(std::to_string(fileIndex), 8, '0') + ".png", img);
      fileIndex++;
    }
  });
#endif

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

  switch(map()->shaderProfile())
  {
    case ShaderProfile::GLSL_110: [[fallthrough]];
    case ShaderProfile::GLSL_120: [[fallthrough]];
    case ShaderProfile::GLSL_130: [[fallthrough]];
    case ShaderProfile::GLSL_140: [[fallthrough]];
    case ShaderProfile::GLSL_150: [[fallthrough]];
    case ShaderProfile::GLSL_330: [[fallthrough]];
    case ShaderProfile::GLSL_400: [[fallthrough]];
    case ShaderProfile::GLSL_410: [[fallthrough]];
    case ShaderProfile::GLSL_420: [[fallthrough]];
    case ShaderProfile::GLSL_430: [[fallthrough]];
    case ShaderProfile::GLSL_440: [[fallthrough]];
    case ShaderProfile::GLSL_450: [[fallthrough]];
    case ShaderProfile::GLSL_460:
    {
      glTexImage2D(target, 0, format, int(img.width()), int(img.height()), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.constData());
      break;
    }

    case ShaderProfile::GLSL_100_ES: [[fallthrough]];
    case ShaderProfile::GLSL_300_ES: [[fallthrough]];
    case ShaderProfile::GLSL_310_ES: [[fallthrough]];
    case ShaderProfile::GLSL_320_ES:
    {
      if(format == GL_RGB)
      {
        // For GL ES we seem to need the internalFormat and format to be the same, so here we take the
        // RGBA data and pack it as RGB.
        struct RGB
        {
          uint8_t r;
          uint8_t g;
          uint8_t b;
        };
        std::vector<RGB> packed;
        packed.resize(img.size());

        auto dst = packed.begin();
        auto src = img.constData();
        for(; dst!=packed.end(); src++, ++dst)
        {
          dst->r = src->r;
          dst->g = src->g;
          dst->b = src->b;
        }

        // Each pixel is 3 bytes so row alignment may not be 4 bytes so set it to 1
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(target, 0, GL_RGB, int(img.width()), int(img.height()), 0, GL_RGB, GL_UNSIGNED_BYTE, packed.data());
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
      }
      else if(format == GL_RGBA)
      {
        glTexImage2D(target, 0, GL_RGBA, int(img.width()), int(img.height()), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.constData());
      }

      break;
    }

    case ShaderProfile::HLSL_10:
    case ShaderProfile::HLSL_11:
    case ShaderProfile::HLSL_12:
    case ShaderProfile::HLSL_13:
    case ShaderProfile::HLSL_14:
    case ShaderProfile::HLSL_20:
    case ShaderProfile::HLSL_20a:
    case ShaderProfile::HLSL_20b:
    case ShaderProfile::HLSL_30:
    case ShaderProfile::HLSL_40:
    case ShaderProfile::HLSL_41:
    case ShaderProfile::HLSL_50:
    case ShaderProfile::HLSL_51:
    case ShaderProfile::HLSL_60:
    case ShaderProfile::HLSL_61:
    case ShaderProfile::HLSL_62:
    case ShaderProfile::HLSL_63:
    case ShaderProfile::HLSL_64:
    case ShaderProfile::HLSL_65:
    case ShaderProfile::HLSL_66:
    case ShaderProfile::HLSL_67:
    {
      tpWarning() << "HLSL not implemented.";
      break;
    }
  }

  if((minFilterOption == GL_NEAREST_MIPMAP_NEAREST) || (minFilterOption == GL_LINEAR_MIPMAP_LINEAR))
    glGenerateMipmap(target);

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, magFilterOption);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, minFilterOption);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, textureWrapS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, textureWrapT);

#if defined(TP_LINUX) && !defined(TP_GLES3)
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
