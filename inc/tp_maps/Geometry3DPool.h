#ifndef tp_maps_Geometry3DPool_h
#define tp_maps_Geometry3DPool_h

#include "tp_maps/Layer.h"
#include "tp_maps/shaders/G3DMaterialShader.h"

#include "tp_math_utils/Geometry3D.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
class Texture;
class TexturePool;

//##################################################################################################
struct ProcessedGeometry3D
{
  TP_REF_COUNT_OBJECTS("ProcessedGeometry3D");
  std::vector<std::pair<GLenum, G3DMaterialShader::VertexBuffer*>> vertexBuffers;
  tp_math_utils::OpenGLMaterial material;
  glm::mat3 materialUVMatrix{1.0f};
  tp_utils::StringID materialName;

  glm::mat3 uvMatrix{1.0f};

  GLuint     rgbaTextureID{0}; //!< Albedo and alpha.
  GLuint  normalsTextureID{0}; //!< Normals.
  GLuint    rmttrTextureID{0}; //!< Roughness, metalness, transmission and transmission roughness.

  ProcessedGeometry3D const* alternativeMaterial{nullptr};
};

//##################################################################################################
typedef std::function<void(const std::vector<ProcessedGeometry3D>&)> ProcessedGeometryCallback;

//##################################################################################################
class TP_MAPS_EXPORT Geometry3DPool
{
  TP_REF_COUNT_OBJECTS("Geometry3DPool");
  TP_NONCOPYABLE(Geometry3DPool);
public:
  //################################################################################################
  Geometry3DPool(Map* map, TexturePool* texturePool=nullptr);

  //################################################################################################
  Geometry3DPool(Layer* layer, TexturePool* texturePool=nullptr);

  //################################################################################################
  ~Geometry3DPool();

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
                             const tp_math_utils::AlternativeMaterials& alternativeMaterials,
                             const std::vector<glm::mat3>& uvMatrices,
                             const ProcessedGeometryCallback& closure);

  //################################################################################################
  void viewGeometry(const tp_utils::StringID& name,
                    const tp_math_utils::GeometryCallback& closure) const;

  //################################################################################################
  void viewGeometry(const tp_utils::StringID& name,
                    const tp_math_utils::AlternativeMaterials& alternativeMaterials,
                    const tp_math_utils::GeometryMaterialCallback& closure) const;

  //################################################################################################
  tp_math_utils::FindGeometry findGeometryFunctor() const;

  //################################################################################################
  tp_utils::CallbackCollection<void()> changed;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
