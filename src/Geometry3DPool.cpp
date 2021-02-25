#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Map.h"
#include "tp_maps/TexturePool.h"
#include "tp_maps/TexturePoolKey.h"
#include "tp_maps/textures/BasicTexture.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{

//##################################################################################################
struct TextureKeys_lt
{
  TexturePoolKey rgba;
  TexturePoolKey normals;
  TexturePoolKey rmao;
};

//##################################################################################################
struct PoolDetails_lt
{
  int count{0};
  bool overwrite{false};
  bool isOnlyMaterial{false};
  std::vector<tp_math_utils::Geometry3D> geometry;
  std::vector<ProcessedGeometry3D> processedGeometry;
  std::vector<TextureKeys_lt> textureKeys;

  bool updateVertexBuffer{true};
  bool updateVertexBufferTextures{true};
  bool bindBeforeRender{true};

  std::unordered_set<TexturePoolKey> textureSubscriptions;

  //################################################################################################
  void deleteVertexBuffers()
  {
    for(const auto& details : processedGeometry)
      for(const auto& buffer : details.vertexBuffers)
        delete buffer.second;

    processedGeometry.clear();
  }

  //################################################################################################
  void checkUpdateVertexBuffer(Geometry3DShader* shader, Map* map)
  {
    if(updateVertexBuffer)
    {
      deleteVertexBuffers();
      updateVertexBuffer=false;
      updateVertexBufferTextures=true;

      for(const auto& shape : geometry)
      {
        for(const auto& part : shape.indexes)
        {
          ProcessedGeometry3D details;
          details.material = shape.material;

          if(!isOnlyMaterial)
          {
            std::vector<GLuint> indexes;
            std::vector<MaterialShader::Vertex> verts;
            for(size_t n=0; n<part.indexes.size(); n++)
            {
              auto idx = part.indexes.at(n);
              if(size_t(idx)<shape.verts.size())
              {
                const auto& v = shape.verts.at(size_t(idx));
                indexes.push_back(GLuint(n));
                verts.emplace_back(MaterialShader::Vertex(v.vert, v.normal, v.texture));
              }
            }

            std::pair<GLenum, MaterialShader::VertexBuffer*> p;
            p.first = GLenum(part.type);
            p.second = shader->generateVertexBuffer(map, indexes, verts);
            details.vertexBuffers.push_back(p);
          }

          processedGeometry.push_back(details);
        }
      }
    }
  }

  //################################################################################################
  void checkUpdateVertexBufferTextures(TexturePool* texturePool)
  {
    if(updateVertexBufferTextures)
    {
      updateVertexBufferTextures=false;
      size_t mMax = tpMin(processedGeometry.size(), textureKeys.size());
      for(size_t m=0; m<mMax; m++)
      {
        auto& details = processedGeometry.at(m);
        const auto& textureDetails = textureKeys.at(m);

        details.    rgbaTextureID = texturePool->textureID(textureDetails.rgba    );
        details. normalsTextureID = texturePool->textureID(textureDetails.normals );
        details.    rmaoTextureID = texturePool->textureID(textureDetails.rmao    );
      }
    }
  }
};
}

//##################################################################################################
struct Geometry3DPool::Private
{
  Geometry3DPool* q;
  Map* m_map;
  Layer* m_layer;

  TexturePool ownedTexturePool;
  TexturePool* texturePool{nullptr};

  std::unordered_map<tp_utils::StringID, PoolDetails_lt> pools;

  //################################################################################################
  Private(Geometry3DPool* q_, Map* map_):
    q(q_),
    m_map(map_),
    m_layer(nullptr),
    ownedTexturePool(m_map)
  {
    invalidateBuffersCallback.connect(m_map->invalidateBuffersCallbacks);
  }

  //################################################################################################
  Private(Geometry3DPool* q_, Layer* layer_):
    q(q_),
    m_map(nullptr),
    m_layer(layer_),
    ownedTexturePool(m_layer)
  {
    invalidateBuffersCallback.connect(m_layer->invalidateBuffersCallbacks);
  }

  //################################################################################################
  ~Private()
  {
    for(auto& i : pools)
      i.second.deleteVertexBuffers();
  }

  //################################################################################################
  Map* map()
  {
    return m_layer?m_layer->map():m_map;
  }

  //################################################################################################
  tp_utils::Callback<void()> texturePoolChanged = [&]
  {
    for(auto& i : pools)
      i.second.updateVertexBufferTextures=true;
    q->changedCallbacks();
  };

  //################################################################################################
  tp_utils::Callback<void()> invalidateBuffersCallback = [&]
  {
    for(auto& i : pools)
    {
      i.second.deleteVertexBuffers();
      i.second.updateVertexBuffer = true;
      i.second.bindBeforeRender = true;
      i.second.updateVertexBufferTextures=true;
    }
  };

  //################################################################################################
  void unsubscribeTextures(std::unordered_set<TexturePoolKey>& textureSubscriptions)
  {
    if(texturePool)
      for(const auto& key : textureSubscriptions)
        texturePool->unsubscribe(key);

    textureSubscriptions.clear();
  }
};

//##################################################################################################
Geometry3DPool::Geometry3DPool(Map* map):
  d(new Private(this, map))
{
  setTexturePool(&d->ownedTexturePool);
}

//##################################################################################################
Geometry3DPool::Geometry3DPool(Layer* layer):
  d(new Private(this, layer))
{

}

//##################################################################################################
Geometry3DPool::~Geometry3DPool()
{
  delete d;
}

//##################################################################################################
void Geometry3DPool::setTexturePool(TexturePool* texturePool)
{
  d->texturePool = texturePool;
  d->texturePoolChanged.disconnect();
  d->texturePoolChanged.connect(texturePool->changedCallbacks);
  d->texturePoolChanged();
}

//##################################################################################################
TexturePool* Geometry3DPool::texturePool() const
{
  return d->texturePool;
}

//##################################################################################################
void Geometry3DPool::subscribe(const tp_utils::StringID& name,
                               const std::function<std::vector<tp_math_utils::Geometry3D>()>& getGeometry,
                               bool overwrite,
                               bool isOnlyMaterial)
{
  auto& details = d->pools[name];
  details.count++;
  if(details.count==1 || overwrite || details.overwrite)
  {
    bool tileTextures=false;

    details.overwrite = false;
    details.geometry = getGeometry();
    details.updateVertexBuffer = true;
    details.isOnlyMaterial = isOnlyMaterial;
    d->unsubscribeTextures(details.textureSubscriptions);

    std::unordered_set<TexturePoolKey> textureSubscriptions;
    details.textureKeys.resize(details.geometry.size());
    for(size_t m=0; m<details.geometry.size(); m++)
    {
      const auto& material = details.geometry.at(m).material;
      auto& textureKeys = details.textureKeys.at(m);

      tileTextures = material.tileTextures;

      textureKeys.rgba     = TexturePoolKey(material.   albedoTexture, material.   albedoTexture, material.  albedoTexture, material.alphaTexture, 0, 1, 2, 0, TPPixel(uint8_t(material.  albedo.x*255.0f), uint8_t(material.  albedo.y*255.0f), uint8_t(material.  albedo.z*255.0f), uint8_t(material.alpha*255.0f)));
      textureKeys.normals  = TexturePoolKey(material.  normalsTexture, material.  normalsTexture, material. normalsTexture,  tp_utils::StringID(), 0, 1, 2, 0, TPPixel(128                                , 128                                , 255                                , 255                           ));
      textureKeys.rmao     = TexturePoolKey(material.roughnessTexture, material.metalnessTexture,     tp_utils::StringID(),  tp_utils::StringID(), 0, 1, 2, 0, TPPixel(uint8_t(material. roughness*255.0f), uint8_t(material. metalness*255.0f), 255                                , 255                           ));

      textureSubscriptions.insert(textureKeys.rgba    );
      textureSubscriptions.insert(textureKeys.normals );
      textureSubscriptions.insert(textureKeys.rmao    );
    }

    details.textureSubscriptions.swap(textureSubscriptions);
    for(const auto& key : details.textureSubscriptions)
    {
      d->texturePool->subscribe(key);
      d->texturePool->setTextureWrapS(key, tileTextures?GL_REPEAT:GL_CLAMP_TO_EDGE);
      d->texturePool->setTextureWrapT(key, tileTextures?GL_REPEAT:GL_CLAMP_TO_EDGE);
    }

    details.updateVertexBufferTextures=true;
  }
  changedCallbacks();
}

//##################################################################################################
void Geometry3DPool::unsubscribe(const tp_utils::StringID& name)
{
  auto i = d->pools.find(name);
  i->second.count--;
  if(!i->second.count)
  {
    i->second.deleteVertexBuffers();
    d->unsubscribeTextures(i->second.textureSubscriptions);
    d->pools.erase(i);
  }
}

//##################################################################################################
void Geometry3DPool::invalidate(const tp_utils::StringID& name)
{
  if(auto i = d->pools.find(name); i != d->pools.end())
    i->second.overwrite = true;
}

//##################################################################################################
void Geometry3DPool::viewProcessedGeometry(const tp_utils::StringID& name,
                                           Geometry3DShader* shader,
                                           const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials,
                                           const std::function<void(const std::vector<ProcessedGeometry3D>&)>& closure)
{
  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  i->second.checkUpdateVertexBuffer(shader, d->map());
  i->second.checkUpdateVertexBufferTextures(d->texturePool);

  if(!i->second.isOnlyMaterial)
  {
    for(auto& mesh : i->second.processedGeometry)
    {
      mesh.alternativeMaterial = &mesh;
      auto itr = alternativeMaterials.find(mesh.material.name);
      if(itr != alternativeMaterials.end())
      {
        viewProcessedGeometry(itr->second, shader, {}, [&](const std::vector<ProcessedGeometry3D>& geometry)
        {
          if(!geometry.empty())
            mesh.alternativeMaterial = &geometry.front();
        });
      }
    }
  }

  closure(i->second.processedGeometry);
}

//##################################################################################################
void Geometry3DPool::viewGeometry(const tp_utils::StringID& name,
                                  const std::function<void(const std::vector<tp_math_utils::Geometry3D>&)>& closure) const
{
  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  closure(i->second.geometry);
}

}
