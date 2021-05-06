#ifndef tp_maps_TexturePool_h
#define tp_maps_TexturePool_h

#include "tp_maps/Globals.h"

#include "tp_utils/CallbackCollection.h"

#include <functional>

namespace tp_image_utils
{
class ColorMap;
}

namespace tp_maps
{
class Map;
class Layer;
class TexturePoolKey;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT TexturePool
{
public:
  //################################################################################################
  TexturePool(Map* map);

  //################################################################################################
  TexturePool(Layer* layer);

  //################################################################################################
  ~TexturePool();

  //################################################################################################
  void incrementKeepHot(bool keepHot);

  //################################################################################################
  void subscribe(const tp_utils::StringID& name,
                 const tp_image_utils::ColorMap& image,
                 bool makeSquare=true);

  //################################################################################################
  void unsubscribe(const tp_utils::StringID& name);

  //################################################################################################
  void subscribe(const TexturePoolKey& key);

  //################################################################################################
  void unsubscribe(const TexturePoolKey& key);

  //################################################################################################
  void invalidate(const tp_utils::StringID& name);

  //################################################################################################
  GLuint textureID(const tp_utils::StringID& name);

  //################################################################################################
  GLuint textureID(const TexturePoolKey& key);

  //################################################################################################
  void setTextureWrapS(const tp_utils::StringID& name, GLint textureWrapS);

  //################################################################################################
  void setTextureWrapT(const tp_utils::StringID& name, GLint textureWrapT);

  //################################################################################################
  void setTextureWrapS(const TexturePoolKey& key, GLint textureWrapS);

  //################################################################################################
  void setTextureWrapT(const TexturePoolKey& key, GLint textureWrapT);

  //################################################################################################
  tp_utils::CallbackCollection<void()> changedCallbacks;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
