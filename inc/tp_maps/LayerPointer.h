#ifndef tp_maps_LayerPointer_h
#define tp_maps_LayerPointer_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Layer;

//##################################################################################################
class TP_MAPS_EXPORT LayerPointer
{
  friend class Layer;
  TP_NONCOPYABLE(LayerPointer);
  Layer* m_layer{nullptr};
public:
  //################################################################################################
  LayerPointer(Layer* layer);

  //################################################################################################
  ~LayerPointer();

  //################################################################################################
  Layer* layer();
};

}

#endif
