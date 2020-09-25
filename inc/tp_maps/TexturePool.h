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
class Layer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT TexturePool
{
public:
  //################################################################################################
  TexturePool(Layer* layer);

  //################################################################################################
  ~TexturePool();

  //################################################################################################
  void subscribe(const tp_utils::StringID& name, const tp_image_utils::ColorMap& image);

  //################################################################################################
  void unsubscribe(const tp_utils::StringID& name);

  //################################################################################################
  GLuint textureID(const tp_utils::StringID& name);

  //################################################################################################
  tp_utils::CallbackCollection<void()> changedCallbacks;

private:
  struct Private;
  friend struct Private;
  Private* d;
};

}

#endif
