#ifndef tp_maps_TexturePoolKey_h
#define tp_maps_TexturePoolKey_h

#include "tp_maps/Globals.h"

#include "tp_utils/TPPixel.h"

namespace tp_maps
{
//##################################################################################################
class TP_MAPS_EXPORT TexturePoolKeyBase
{
public:
  tp_utils::StringID rName;
  tp_utils::StringID gName;
  tp_utils::StringID bName;
  tp_utils::StringID aName;
  size_t rIndex;
  size_t gIndex;
  size_t bIndex;
  size_t aIndex;

  size_t h;

  TPPixel defaultColor;

  NChannels nChannels;
};

//##################################################################################################
class TP_MAPS_EXPORT TexturePoolKey : private TexturePoolKeyBase
{
  friend bool operator==(const TexturePoolKey& a, const TexturePoolKey& b);
  friend bool operator!=(const TexturePoolKey& a, const TexturePoolKey& b);
  friend struct ::std::hash<tp_maps::TexturePoolKey>;
public:
  //################################################################################################
  TexturePoolKey();

  //################################################################################################
  TexturePoolKey(const tp_utils::StringID& rName_,
                 const tp_utils::StringID& gName_,
                 const tp_utils::StringID& bName_,
                 const tp_utils::StringID& aName_,
                 size_t rIndex_,
                 size_t gIndex_,
                 size_t bIndex_,
                 size_t aIndex_,
                 TPPixel defaultColor_,
                 NChannels nChannels_);

  //################################################################################################
  size_t makeHash();

  //################################################################################################
  inline const TexturePoolKeyBase& d() const{return *this;}

  //################################################################################################
  std::string debugString() const;
};

//##################################################################################################
bool operator==(const TexturePoolKey& a, const TexturePoolKey& b);

//##################################################################################################
bool operator!=(const TexturePoolKey& a, const TexturePoolKey& b);

}

namespace std
{
//##################################################################################################
template <>
struct hash<tp_maps::TexturePoolKey>
{
  size_t operator()(const tp_maps::TexturePoolKey& key) const
  {
    return key.h;
  }
};
}

#endif
