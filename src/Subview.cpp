#include "tp_maps/Subview.h"

namespace tp_maps
{
class Map;

//##################################################################################################
Subview::Subview(const tp_utils::StringID& name):
  m_name(name)
{

}

//##################################################################################################
Subview::~Subview()
{

}

//##################################################################################################
const tp_utils::StringID& Subview::name() const
{
  return m_name;
}

}
