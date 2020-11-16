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
  Material material;

  GLuint ambientTextureID{0};
  GLuint diffuseTextureID{0};
  GLuint specularTextureID{0};
  GLuint alphaTextureID{0};
  GLuint bumpTextureID{0};
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Geometry3DPool
{
  TP_REF_COUNT_OBJECTS("Geometry3DPool");
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
  void subscribe(const tp_utils::StringID& name,
                 const std::function<std::vector<Geometry3D>()>& getGeometry,
                 bool overwrite);

  //################################################################################################
  void unsubscribe(const tp_utils::StringID& name);

  //################################################################################################
  void invalidate(const tp_utils::StringID& name);

  //################################################################################################
  void viewProcessedGeometry(const tp_utils::StringID& name,
                             Geometry3DShader* shader,
                             const std::function<void(const std::vector<ProcessedGeometry3D>&)>& closure);

  //################################################################################################
  void viewGeometry(const tp_utils::StringID& name,
                    const std::function<void(const std::vector<Geometry3D>&)>& closure) const;
  
  
  
  //  //################################################################################################
  //  const std::vector<Geometry3D>& geometry() const;
  
  //  //################################################################################################
  //  void setGeometry(const std::vector<Geometry3D>& geometry);
  
  //  //################################################################################################
  //  //! Call this to set the material of all geometry.
  //  void setMaterial(const Material& material);

  //################################################################################################
  tp_utils::CallbackCollection<void()> changedCallbacks;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
