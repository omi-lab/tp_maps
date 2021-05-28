#ifndef tp_maps_Geometry3DPool_h
#define tp_maps_Geometry3DPool_h

#include "tp_maps/Layer.h"
#include "tp_maps/shaders/MaterialShader.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
class Texture;
class TexturePool;

//##################################################################################################
struct ProcessedGeometry3D
{
  TP_REF_COUNT_OBJECTS("ProcessedGeometry3D");
  std::vector<std::pair<GLenum, MaterialShader::VertexBuffer*>> vertexBuffers;
  tp_math_utils::Material material;

  GLuint     rgbaTextureID{0}; //!< Albedo and alpha.
  GLuint  normalsTextureID{0}; //!< Normals.
  GLuint     rmaoTextureID{0}; //!< Roughness, metalness, and ambient occlusion.

  ProcessedGeometry3D const* alternativeMaterial{nullptr};
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Geometry3DPool
{
  TP_REF_COUNT_OBJECTS("Geometry3DPool");
  TP_NONCOPYABLE(Geometry3DPool);
public:
  //################################################################################################
  Geometry3DPool(Map* map);

  //################################################################################################
  Geometry3DPool(Layer* layer);

  //################################################################################################
  ~Geometry3DPool();

  //################################################################################################
  void setTexturePool(TexturePool* texturePool);

  //################################################################################################
  TexturePool* texturePool() const;

  //################################################################################################
  void incrementKeepHot(bool keepHot);

  //################################################################################################
  //! Add geometry and material to pool
  /*!
  \param name used to index the geometry/material.
  \param getGeometry used to fetch geometry if not already in the pool.
  \param overwrite what we have in the pool already.
  \param isOnlyMaterial if true don't generate vertex buffers.
  */
  void subscribe(const tp_utils::StringID& name,
                 const std::function<std::vector<tp_math_utils::Geometry3D>()>& getGeometry,
                 bool overwrite,
                 bool isOnlyMaterial=false);

  //################################################################################################
  void unsubscribe(const tp_utils::StringID& name);

  //################################################################################################
  void invalidate(const tp_utils::StringID& name);

  //################################################################################################
  void viewProcessedGeometry(const tp_utils::StringID& name,
                             Geometry3DShader* shader,
                             const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials,
                             const std::function<void(const std::vector<ProcessedGeometry3D>&)>& closure);

  //################################################################################################
  void viewGeometry(const tp_utils::StringID& name,
                    const std::function<void(const std::vector<tp_math_utils::Geometry3D>&)>& closure) const;

  //################################################################################################
  void viewGeometry(const tp_utils::StringID& name,
                    const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials,
                    const std::function<void(const std::vector<tp_math_utils::Geometry3D>&, const std::vector<tp_math_utils::Material>&)>& closure) const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> changedCallbacks;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
