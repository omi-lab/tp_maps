#ifndef tp_maps_Subview_h
#define tp_maps_Subview_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;
class Controller;

//##################################################################################################
//! A sub view within the Map.
class TP_MAPS_EXPORT Subview
{
  friend class Map;
  TP_NONCOPYABLE(Subview);
public:
  //################################################################################################
  Subview(Map* map, const tp_utils::StringID& name);

  //################################################################################################
  ~Subview();

  //################################################################################################
  const tp_utils::StringID& name() const;

  //################################################################################################
  Controller* controller() const;

  //################################################################################################
  size_t width() const;

  //################################################################################################
  size_t height() const;

private:  
  //################################################################################################
  void setRenderPassesInternal(const std::vector<RenderPass>& renderPasses);

  //################################################################################################
  bool hasRenderPass(RenderPass::RenderPassType renderPassType);

  //################################################################################################
  void deletePostLayers();

  Map* m_map;
  const tp_utils::StringID m_name;

  std::vector<RenderPass> m_renderPasses;
  std::vector<RenderPass> m_computedRenderPasses;

  Controller* m_controller{nullptr};

  size_t m_width{1};
  size_t m_height{1};

  RenderFromStage m_renderFromStage{RenderFromStage::Full};
};

//##################################################################################################
struct FBOKey
{
  const tp_utils::StringID name;
  const Subview* subview;
  const size_t hash;

  //################################################################################################
  FBOKey(const FBOKey& other) = default;

  //################################################################################################
  FBOKey(FBOKey&& other) noexcept = default;

  //################################################################################################
  FBOKey(const tp_utils::StringID& name_, Subview* subview_):
    name(name_),
    subview(subview_),
    hash(calcHash(name_, subview_))
  {

  }

  //################################################################################################
  static size_t calcHash(const tp_utils::StringID& name, Subview* subview)
  {
    size_t hash = std::hash<tp_utils::StringID> ()(name);
    hash ^= std::hash<void*>()(subview ) + 0x9e3779b9 + (hash<<6) + (hash>>2);
    return hash;
  }

  friend bool operator==(const FBOKey& a, const FBOKey& b);
  friend bool operator!=(const FBOKey& a, const FBOKey& b);
};

//##################################################################################################
inline bool operator==(const FBOKey& a, const FBOKey& b)
{
  return
      a.hash    == b.hash &&
      a.name    == b.name &&
      a.subview == b.subview;
}

//##################################################################################################
inline bool operator!=(const FBOKey& a, const FBOKey& b)
{
  return
      a.hash    != b.hash ||
      a.name    != b.name ||
      a.subview != b.subview;
}

}

namespace std
{
//##################################################################################################
template <>
struct hash<tp_maps::FBOKey>
{
  size_t operator()(const tp_maps::FBOKey& key) const
  {
    return key.hash;
  }
};
}

#endif
