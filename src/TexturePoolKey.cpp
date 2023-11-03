#include "tp_maps/TexturePoolKey.h"

namespace tp_maps
{

//##################################################################################################
TexturePoolKey::TexturePoolKey()
{
  rIndex = 0;
  gIndex = 0;
  bIndex = 0;
  aIndex = 0;

  h = 0;
}

//##################################################################################################
TexturePoolKey::TexturePoolKey(const tp_utils::StringID& rName_,
                               const tp_utils::StringID& gName_,
                               const tp_utils::StringID& bName_,
                               const tp_utils::StringID& aName_,
                               size_t rIndex_,
                               size_t gIndex_,
                               size_t bIndex_,
                               size_t aIndex_,
                               TPPixel defaultColor_,
                               NChannels nChannels_)
{
  rName = rName_;
  gName = gName_;
  bName = bName_;
  aName = aName_;

  rIndex = rIndex_;
  gIndex = gIndex_;
  bIndex = bIndex_;
  aIndex = aIndex_;

  defaultColor = defaultColor_;

  nChannels = nChannels_;

  h = makeHash();
}

//##################################################################################################
size_t TexturePoolKey::makeHash()
{
  size_t h =  std::hash<tp_utils::StringID>()(rName);
  h ^= std::hash<tp_utils::StringID>()(gName) + 0x9e3779b9 + (h<<6) + (h>>2);
  h ^= std::hash<tp_utils::StringID>()(bName) + 0x9e3779b9 + (h<<6) + (h>>2);
  h ^= std::hash<tp_utils::StringID>()(aName) + 0x9e3779b9 + (h<<6) + (h>>2);

  h ^= std::hash<size_t>()(rIndex) + 0x9e3779b9 + (h<<6) + (h>>2);
  h ^= std::hash<size_t>()(gIndex) + 0x9e3779b9 + (h<<6) + (h>>2);
  h ^= std::hash<size_t>()(bIndex) + 0x9e3779b9 + (h<<6) + (h>>2);
  h ^= std::hash<size_t>()(aIndex) + 0x9e3779b9 + (h<<6) + (h>>2);

  h ^= std::hash<size_t>()(defaultColor.i) + 0x9e3779b9 + (h<<6) + (h>>2);

  h ^= std::hash<size_t>()(size_t(nChannels)) + 0x9e3779b9 + (h<<6) + (h>>2);

  return h;
}

//##################################################################################################
std::string TexturePoolKey::debugString() const
{
  std::string s;

  s += "rName: " + rName.toString();
  s += " gName: " + gName.toString();
  s += " bName: " + bName.toString();
  s += " aName: " + aName.toString();

  s += " rIndex: " + std::to_string(rIndex);
  s += " gIndex: " + std::to_string(gIndex);
  s += " bIndex: " + std::to_string(bIndex);
  s += " aIndex: " + std::to_string(aIndex);

  s += " defaultColor: " + defaultColor.toString();
  s += " nChannels: " + std::to_string(size_t(nChannels));
  s += " h: " + std::to_string(h);

  return s;
}

//##################################################################################################
bool operator==(const TexturePoolKey& a, const TexturePoolKey& b)
{
  if(a.h!=b.h)return false;

  if(a.rName!=b.rName)return false;
  if(a.gName!=b.gName)return false;
  if(a.bName!=b.bName)return false;
  if(a.aName!=b.aName)return false;

  if(a.rIndex!=b.rIndex)return false;
  if(a.gIndex!=b.gIndex)return false;
  if(a.bIndex!=b.bIndex)return false;
  if(a.aIndex!=b.aIndex)return false;

  if(a.defaultColor!=b.defaultColor)return false;

  return true;
}

//##################################################################################################
bool operator!=(const TexturePoolKey& a, const TexturePoolKey& b)
{
  return !(a==b);
}

}
