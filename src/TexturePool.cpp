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
  Layer* layer;
  std::unordered_map<tp_utils::StringID, Details_lt> images;

  //################################################################################################
  Private(Layer* layer_):
    layer(layer_)
  {
    invalidateBuffersCallback.connect(layer->invalidateBuffersCallbacks);
  }

  //################################################################################################
  ~Private()
  {
    if(layer->map())
    {
      layer->map()->makeCurrent();
      for(auto& i : images)
        layer->map()->deleteTexture(i.second.textureID);
    }
  }

  //################################################################################################
  tp_utils::Callback<void()> invalidateBuffersCallback = [&]
  {
    for(auto& i : images)
      i.second.textureID=0;
  };
};

//##################################################################################################
TexturePool::TexturePool(Layer* layer):
  d(new Private(layer))
{

}

//##################################################################################################
TexturePool::~TexturePool()
{
  delete d;
}


// //##################################################################################################
// void Geometry3DLayer::setTextures(const std::unordered_map<tp_utils::StringID, Texture*>& textures)
// {
//   for(const auto& i : d->textures)
//     delete i.second;
//   d->textures = textures;
//
//   for(const auto& i : d->textures)
//   {
//     i.second->setImageChangedCallback([this]()
//     {
//       d->bindBeforeRender = true;
//       update();
//     });
//   }
//   d->bindBeforeRender = true;
//   update();
// }


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
    if(i->second.textureID && d->layer->map())
      d->layer->map()->deleteTexture(i->second.textureID);
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
    i->second.texture = new BasicTexture(d->layer->map(), i->second.image);

  if(!i->second.textureID)
    i->second.textureID = i->second.texture->bindTexture();

  return i->second.textureID;
}

}
