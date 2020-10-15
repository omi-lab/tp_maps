#include "tp_maps/TexturePool.h"
#include "tp_maps/Layer.h"
#include "tp_maps/Map.h"
#include "tp_maps/textures/BasicTexture.h"

#include "tp_image_utils/ColorMap.h"

namespace tp_maps
{

namespace
{
struct Details_lt
{
  int count{0};
  tp_image_utils::ColorMap image;
  BasicTexture* texture{nullptr};
  GLuint textureID{0};
};
}

//##################################################################################################
struct TexturePool::Private
{
  Map* m_map;
  Layer* m_layer;
  std::unordered_map<tp_utils::StringID, Details_lt> images;

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
    }
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
  auto& details = d->images[name];
  details.count++;
  if(!details.texture)
  {
    details.image = image;
    changedCallbacks();
  }
}

//##################################################################################################
void TexturePool::unsubscribe(const tp_utils::StringID& name)
{
  auto i = d->images.find(name);
  i->second.count--;
  if(!i->second.count)
  {
    if(i->second.textureID && d->map())
      d->map()->deleteTexture(i->second.textureID);
    d->images.erase(i);
  }
}

//##################################################################################################
GLuint TexturePool::textureID(const tp_utils::StringID& name)
{
  auto i = d->images.find(name);
  if(i == d->images.end())
    return 0;

  if(!i->second.texture)
    i->second.texture = new BasicTexture(d->map(), i->second.image);

  if(!i->second.textureID)
    i->second.textureID = i->second.texture->bindTexture();

  return i->second.textureID;
}

}
