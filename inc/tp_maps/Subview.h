#ifndef tp_maps_Subview_h
#define tp_maps_Subview_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;

//##################################################################################################
//! A sub view within the Map.
class TP_MAPS_EXPORT Subview
{
  friend class Map;
  TP_NONCOPYABLE(Subview);
public:
  //################################################################################################
  Subview(const tp_utils::StringID& name);

  //################################################################################################
  ~Subview();

  //################################################################################################
  const tp_utils::StringID& name() const;

private:
  const tp_utils::StringID m_name;
};

}

#endif
