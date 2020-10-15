#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Map.h"
#include "tp_maps/TexturePool.h"
#include "tp_maps/textures/BasicTexture.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
//##################################################################################################
struct PoolDetails_lt
{
  int count{0};
  bool overwrite{false};
  std::vector<Geometry3D> geometry;
  std::vector<ProcessedGeometry3D> processedGeometry;

  bool updateVertexBuffer{true};
  bool updateVertexBufferTextures{true};
  bool bindBeforeRender{true};

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
        for(const auto& part : shape.geometry.indexes)
        {
          ProcessedGeometry3D details;
          details.material = shape.material;

          std::vector<GLuint> indexes;
          std::vector<MaterialShader::Vertex> verts;
          for(size_t n=0; n<part.indexes.size(); n++)
          {
            auto idx = part.indexes.at(n);
            if(size_t(idx)<shape.geometry.verts.size())
            {
              const auto& v = shape.geometry.verts.at(size_t(idx));
              indexes.push_back(GLuint(n));
              verts.emplace_back(MaterialShader::Vertex(v.vert, v.normal, v.tangent, v.bitangent, v.texture));
            }
          }

          std::pair<GLenum, MaterialShader::VertexBuffer*> p;
          p.first = GLenum(part.type);
          p.second = shader->generateVertexBuffer(map, indexes, verts);
          details.vertexBuffers.push_back(p);

          processedGeometry.push_back(details);
        }
      }
    }
  }

  //################################################################################################
  void checkUpdateVertexBufferTextures(TexturePool* texturePool, GLuint emptyTextureID, GLuint emptyNormalTextureID)
  {
    if(updateVertexBufferTextures)
    {
      updateVertexBufferTextures=false;

      for(auto& details : processedGeometry)
      {
        auto selectTexture = [&](const tp_utils::StringID& name, GLuint& textureID, glm::vec3& color, GLuint emptyID)
        {
          if(texturePool)
          {
            textureID = texturePool->textureID(name);
            if(textureID == 0)
              textureID = emptyID;
            else
              color = {0.0f, 0.0f, 0.0f};
          }
          else
            textureID = emptyID;
        };

        glm::vec3 tmpNormal{0.0f, 0.0f, 1.0f};
        selectTexture(details.material.ambientTexture, details.ambientTextureID, details.material.ambient, emptyTextureID);
        selectTexture(details.material.diffuseTexture, details.diffuseTextureID, details.material.diffuse, emptyTextureID);
        selectTexture(details.material.specularTexture, details.specularTextureID, details.material.specular, emptyTextureID);
        details.alphaTextureID = emptyTextureID;
        selectTexture(details.material.bumpTexture, details.bumpTextureID, tmpNormal, emptyNormalTextureID);
        //details.bumpTextureID = emptyNormalTextureID;
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

  std::unique_ptr<BasicTexture> emptyTexture;
  std::unique_ptr<BasicTexture> emptyNormalTexture;

  GLuint emptyTextureID{0};
  GLuint emptyNormalTextureID{0};

  bool bindBeforeRender{true};

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

    map()->deleteTexture(emptyTextureID);
    map()->deleteTexture(emptyNormalTextureID);
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

    emptyTextureID = 0;
    emptyNormalTextureID = 0;
  };

  //################################################################################################
  void checkBindTextures()
  {
    if(bindBeforeRender)
    {
      bindBeforeRender=false;

      for(auto& i : pools)
        i.second.updateVertexBufferTextures=true;

      if(!emptyTexture)
      {
        ;
        tp_image_utils::ColorMap textureData{1, 1, nullptr, TPPixel{0, 0, 0, 255}};
        emptyTexture = std::make_unique<BasicTexture>(map(), textureData);
      }

      if(!emptyNormalTexture)
      {
        tp_image_utils::ColorMap textureData{1, 1, nullptr, TPPixel{127, 127, 255, 255}};
        emptyNormalTexture = std::make_unique<BasicTexture>(map(), textureData);
      }

      emptyTextureID = emptyTexture->bindTexture();
      emptyNormalTextureID = emptyNormalTexture->bindTexture();
    }
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
                               const std::vector<Geometry3D>& geometry,
                               bool overwrite)
{
  auto& details = d->pools[name];
  details.count++;
  if(details.count==1 || overwrite || details.overwrite)
  {
    details.overwrite = false;
    details.geometry = geometry;
    details.updateVertexBuffer = true;
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
                                           const std::function<void(const std::vector<ProcessedGeometry3D>&)>& closure)
{
  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  d->checkBindTextures();
  i->second.checkUpdateVertexBuffer(shader, d->map());
  i->second.checkUpdateVertexBufferTextures(d->texturePool, d->emptyTextureID, d->emptyNormalTextureID);

  closure(i->second.processedGeometry);
}

//##################################################################################################
void Geometry3DPool::viewGeometry(const tp_utils::StringID& name,
                                  const std::function<void(const std::vector<Geometry3D>&)>& closure) const
{
  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  closure(i->second.geometry);
}

}
