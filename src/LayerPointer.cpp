#include "tp_maps/LayerPointer.h"
#include "tp_maps/Layer.h"

namespace tp_maps
{

//##################################################################################################
LayerPointer::LayerPointer(Layer* layer):
  m_layer(layer)
{
  if(m_layer)
    m_layer->addPointer(this);
}

//##################################################################################################
LayerPointer::~LayerPointer()
{
  if(m_layer)
    m_layer->removePointer(this);
}

//##################################################################################################
Layer* LayerPointer::layer()
{
  return m_layer;
}

}
