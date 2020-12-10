#include "tp_maps/TexturePool.h"
#include "tp_maps/TexturePoolKey.h"
#include "tp_maps/Layer.h"
#include "tp_maps/Map.h"
#include "tp_maps/textures/BasicTexture.h"

#include "tp_image_utils/ColorMap.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/RefCount.h"

namespace tp_maps
{

namespace
{
//##################################################################################################
struct Details_lt
{
  TP_REF_COUNT_OBJECTS("TexturePool::Details_lt");
  int count{0};
  tp_image_utils::ColorMap image;
  BasicTexture* texture{nullptr};
  bool overwrite{false};
  GLuint textureID{0};

  GLint textureWrapS{GL_CLAMP_TO_EDGE};
  GLint textureWrapT{GL_CLAMP_TO_EDGE};
};

//##################################################################################################
struct CombinedDetails_lt
{
  TP_REF_COUNT_OBJECTS("TexturePool::CombinedDetails_lt");
  int count{0};

  tp_image_utils::ColorMap rImage;
  tp_image_utils::ColorMap gImage;
  tp_image_utils::ColorMap bImage;
  tp_image_utils::ColorMap aImage;

  tp_image_utils::ColorMap rgbaImage;
  bool composeImage{true};

  BasicTexture* texture{nullptr};
  GLuint textureID{0};

  GLint textureWrapS{GL_CLAMP_TO_EDGE};
  GLint textureWrapT{GL_CLAMP_TO_EDGE};
};
}

//##################################################################################################
struct TexturePool::Private
{
  Map* m_map;
  Layer* m_layer;
  std::unordered_map<tp_utils::StringID, Details_lt> images;
  std::unordered_map<TexturePoolKey, CombinedDetails_lt> combinedImages;

  //################################################################################################
  Private(Map* map_, Layer* layer_):
    m_map(map_),
    m_layer(layer_)
  {
    if(m_map)
      invalidateBuffersCallback.connect(m_map->invalidateBuffersCallbacks);
    else if(m_layer)
      invalidateBuffersCallback.connect(m_layer->invalidateBuffersCallbacks);
  }

  //################################################################################################
  ~Private()
  {
    if(map())
    {
      map()->makeCurrent();

      for(auto& i : images)
        map()->deleteTexture(i.second.textureID);

      for(auto& i : combinedImages)
        map()->deleteTexture(i.second.textureID);
    }

    for(auto& i : images)
      delete i.second.texture;

    for(auto& i : combinedImages)
      delete i.second.texture;
  }

  //################################################################################################
  Map* map()
  {
    return (m_layer && m_layer->map())?m_layer->map():m_map;
  }

  //################################################################################################
  tp_utils::Callback<void()> invalidateBuffersCallback = [&]
  {
    for(auto& i : images)
      i.second.textureID=0;

    for(auto& i : combinedImages)
      i.second.textureID=0;
  };
};

//##################################################################################################
TexturePool::TexturePool(Map* map):
  d(new Private(map, nullptr))
{

}

//##################################################################################################
TexturePool::TexturePool(Layer* layer):
  d(new Private(nullptr, layer))
{

}

//##################################################################################################
TexturePool::~TexturePool()
{
  delete d;
}

//##################################################################################################
void TexturePool::subscribe(const tp_utils::StringID& name, const tp_image_utils::ColorMap& image)
{
  {
    auto& details = d->images[name];
    details.count++;

    if(details.overwrite)
    {
      details.overwrite = false;

      if(details.textureID && d->map())
      {
        d->map()->makeCurrent();
        d->map()->deleteTexture(details.textureID);
      }
      delete details.texture;

      details.textureID = 0;
      details.texture = nullptr;
    }

    if(!details.texture)
    {
      details.image = image;
      for(auto& i : d->combinedImages)
      {
        auto& combinedDetails = i.second;
        const auto& key = i.first;

        bool changed=false;
        if(key.d().rName == name){combinedDetails.rImage = image; changed=true;}
        if(key.d().gName == name){combinedDetails.gImage = image; changed=true;}
        if(key.d().bName == name){combinedDetails.bImage = image; changed=true;}
        if(key.d().aName == name){combinedDetails.aImage = image; changed=true;}

        if(changed)
        {
          combinedDetails.composeImage = true;
          if(combinedDetails.textureID && d->map())
          {
            d->map()->makeCurrent();
            d->map()->deleteTexture(combinedDetails.textureID);
          }
          delete combinedDetails.texture;

          combinedDetails.textureID = 0;
          combinedDetails.texture = nullptr;
        }
      }
      changedCallbacks();
    }
  }
}

//##################################################################################################
void TexturePool::unsubscribe(const tp_utils::StringID& name)
{
  auto i = d->images.find(name);
  if(i == d->images.end())
  {
    tpWarning() << "Error TexturePool::unsubscribe(name)!";
    return;
  }

  i->second.count--;
  if(!i->second.count)
  {
    if(i->second.textureID && d->map())
    {
      d->map()->makeCurrent();
      d->map()->deleteTexture(i->second.textureID);
    }
    delete i->second.texture;

    d->images.erase(i);
  }
}

//##################################################################################################
void TexturePool::subscribe(const TexturePoolKey& key)
{
  auto& details = d->combinedImages[key];
  details.count++;

  auto findImage = [&](tp_image_utils::ColorMap& image, const tp_utils::StringID& name)
  {
    if(!name.isValid())
      return;

    auto i = d->images.find(name);
    if(i == d->images.end())
      return;

    image = i->second.image;
  };

  findImage(details.rImage, key.d().rName);
  findImage(details.gImage, key.d().gName);
  findImage(details.bImage, key.d().bName);
  findImage(details.aImage, key.d().aName);
}

//##################################################################################################
void TexturePool::unsubscribe(const TexturePoolKey& key)
{
  auto i = d->combinedImages.find(key);
  if(i == d->combinedImages.end())
  {
    tpWarning() << "Error TexturePool::unsubscribe(key)!";
    return;
  }

  i->second.count--;
  if(!i->second.count)
  {
    if(i->second.textureID && d->map())
    {
      d->map()->makeCurrent();
      d->map()->deleteTexture(i->second.textureID);
    }
    delete i->second.texture;

    d->combinedImages.erase(i);
  }
}

//##################################################################################################
void TexturePool::invalidate(const tp_utils::StringID& name)
{
  if(auto i = d->images.find(name); i != d->images.end())
    i->second.overwrite = true;
}

//##################################################################################################
GLuint TexturePool::textureID(const tp_utils::StringID& name)
{
  auto i = d->images.find(name);
  if(i == d->images.end())
    return 0;

  if(!i->second.texture)
  {
    i->second.texture = new BasicTexture(d->map(), i->second.image);
    i->second.texture->setTextureWrapS(i->second.textureWrapS);
    i->second.texture->setTextureWrapT(i->second.textureWrapT);
  }

  if(!i->second.textureID)
    i->second.textureID = i->second.texture->bindTexture();

  return i->second.textureID;
}

//##################################################################################################
GLuint TexturePool::textureID(const TexturePoolKey& key)
{
  auto i = d->combinedImages.find(key);
  if(i == d->combinedImages.end())
    return 0;

  if(!i->second.texture)
  {
    if(i->second.composeImage)
    {
      i->second.composeImage = false;

      const auto& rImage = i->second.rImage;
      const auto& gImage = i->second.gImage;
      const auto& bImage = i->second.bImage;
      const auto& aImage = i->second.aImage;

      auto rIndex = key.d().rIndex;
      auto gIndex = key.d().gIndex;
      auto bIndex = key.d().bIndex;
      auto aIndex = key.d().aIndex;

      auto defaultColor = key.d().defaultColor;

      size_t w=1;
      size_t h=1;

      w = tpMax(w, rImage.width()); h = tpMax(h, rImage.height());
      w = tpMax(w, gImage.width()); h = tpMax(h, gImage.height());
      w = tpMax(w, bImage.width()); h = tpMax(h, bImage.height());
      w = tpMax(w, aImage.width()); h = tpMax(h, aImage.height());

      i->second.rgbaImage.setSize(w, h);


      for(size_t y=0; y<h; y++)
      {
        for(size_t x=0; x<w; x++)
        {
          TPPixel p;

          auto getChannel = [&](const tp_image_utils::ColorMap& image, size_t index, uint8_t defaultValue)
          {
            if(x<image.width() && y<image.height())
            {
              auto c = image.pixel(x, y);
              switch(index)
              {
              case 0: return c.r;
              case 1: return c.g;
              case 2: return c.b;
              case 3: return c.a;
              }
            }
            return defaultValue;
          };

          p.r = getChannel(rImage, rIndex, defaultColor.r);
          p.g = getChannel(gImage, gIndex, defaultColor.g);
          p.b = getChannel(bImage, bIndex, defaultColor.b);
          p.a = getChannel(aImage, aIndex, defaultColor.a);

          i->second.rgbaImage.setPixel(x, y, p);
        }
      }
    }

    i->second.texture = new BasicTexture(d->map(), i->second.rgbaImage);
    i->second.texture->setTextureWrapS(i->second.textureWrapS);
    i->second.texture->setTextureWrapT(i->second.textureWrapT);
  }

  if(!i->second.textureID)
    i->second.textureID = i->second.texture->bindTexture();

  return i->second.textureID;
}

//##################################################################################################
void TexturePool::setTextureWrapS(const tp_utils::StringID& name, GLint textureWrapS)
{
  auto i = d->images.find(name);
  if(i == d->images.end())
    return;

  if(i->second.textureWrapS == textureWrapS)
    return;

  i->second.textureWrapS = textureWrapS;

  if(i->second.texture)
    i->second.texture->setTextureWrapS(i->second.textureWrapS);

  if(i->second.textureID && d->map())
  {
    d->map()->makeCurrent();
    d->map()->deleteTexture(i->second.textureID);
    i->second.textureID = 0;
    changedCallbacks();
  }
}

//##################################################################################################
void TexturePool::setTextureWrapT(const tp_utils::StringID& name, GLint textureWrapT)
{
  auto i = d->images.find(name);
  if(i == d->images.end())
    return;

  if(i->second.textureWrapT == textureWrapT)
    return;

  i->second.textureWrapT = textureWrapT;

  if(i->second.texture)
    i->second.texture->setTextureWrapT(i->second.textureWrapT);

  if(i->second.textureID && d->map())
  {
    d->map()->makeCurrent();
    d->map()->deleteTexture(i->second.textureID);
    i->second.textureID = 0;
    changedCallbacks();
  }
}

//##################################################################################################
void TexturePool::setTextureWrapS(const TexturePoolKey& key, GLint textureWrapS)
{
  auto i = d->combinedImages.find(key);
  if(i == d->combinedImages.end())
    return;

  if(i->second.textureWrapS == textureWrapS)
    return;

  i->second.textureWrapS = textureWrapS;

  if(i->second.texture)
    i->second.texture->setTextureWrapS(i->second.textureWrapS);

  if(i->second.textureID && d->map())
  {
    d->map()->makeCurrent();
    d->map()->deleteTexture(i->second.textureID);
    i->second.textureID = 0;
    changedCallbacks();
  }
}

//##################################################################################################
void TexturePool::setTextureWrapT(const TexturePoolKey& key, GLint textureWrapT)
{
  auto i = d->combinedImages.find(key);
  if(i == d->combinedImages.end())
    return;

  if(i->second.textureWrapT == textureWrapT)
    return;

  i->second.textureWrapT = textureWrapT;

  if(i->second.texture)
    i->second.texture->setTextureWrapT(i->second.textureWrapT);

  if(i->second.textureID && d->map())
  {
    d->map()->makeCurrent();
    d->map()->deleteTexture(i->second.textureID);
    i->second.textureID = 0;
    changedCallbacks();
  }
}

}
