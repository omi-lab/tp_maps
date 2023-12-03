#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Map.h"
#include "tp_maps/TexturePool.h"
#include "tp_maps/TexturePoolKey.h"

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
  TexturePoolKey rmttr;
};

//##################################################################################################
struct PoolDetails_lt
{
  int count{0};
  bool isNew{true};
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
    TP_FUNCTION_TIME("Geometry3DPool::PoolDetails_lt::deleteVertexBuffers");

    for(const auto& details : processedGeometry)
      for(const auto& buffer : details.vertexBuffers)
        delete buffer.second;

    processedGeometry.clear();
  }

  //################################################################################################
  void checkUpdateVertexBuffer(Geometry3DShader* shader, Map* map)
  {
    TP_FUNCTION_TIME("Geometry3DPool::PoolDetails_lt::checkUpdateVertexBuffer");

    if(updateVertexBuffer)
    {
      deleteVertexBuffers();
      updateVertexBuffer=false;
      updateVertexBufferTextures=true;

      for(const auto& shape : geometry)
      {
        // build tangent vectors for each vertex
        std::vector<glm::vec3> tangent;
        if(!isOnlyMaterial)
          shape.buildTangentVectors(tangent);

        for(const auto& part : shape.indexes)
        {
          ProcessedGeometry3D details;
          details.material = shape.material;

          if(!isOnlyMaterial)
          {
            std::vector<GLuint> indexes;
            std::vector<G3DMaterialShader::Vertex> verts;

            indexes.reserve(part.indexes.size());
            verts.reserve(part.indexes.size());

            for(size_t n=0; n<part.indexes.size(); n++)
            {
              auto idx = size_t(part.indexes.at(n));
              if(idx<shape.verts.size())
              {
                const auto& v = shape.verts.at(idx);
                indexes.push_back(GLuint(n));
                auto tbnq = glm::quatLookAtLH(v.normal, tangent.at(idx));
                verts.emplace_back(G3DMaterialShader::Vertex(v.vert, tbnq, v.texture));
                if(glm::abs(glm::dot(v.normal, tangent.at(idx))) > 0.99f)
                  tpDebug() << "Inconsistent normal & tangent";

#if 0
                // remove once artefacts from tangent calculation ENG-472 have been cleared
                {
                  glm::mat3 R = glm::mat3_cast(tbnq);
                  tpDebug() << "tbnq= (" << tbnq.x << " " << tbnq.y << " " << tbnq.z << " " << tbnq.w << ") sizeof=" << sizeof(tbnq);

                  // check against calculation in shader
                  auto quaternionToMat3 = [](const glm::quat& q)
                  {
                    float qxx = q.x * q.x;
                    float qyy = q.y * q.y;
                    float qzz = q.z * q.z;
                    float qxz = q.x * q.z;
                    float qxy = q.x * q.y;
                    float qyz = q.y * q.z;
                    float qwx = q.w * q.x;
                    float qwy = q.w * q.y;
                    float qwz = q.w * q.z;

                    glm::mat3 R;
                    R[0][0] = 1.0f - 2.0f * (qyy +  qzz);
                    R[0][1] = 2.0f * (qxy + qwz);
                    R[0][2] = 2.0f * (qxz - qwy);

                    R[1][0] = 2.0f * (qxy - qwz);
                    R[1][1] = 1.0f - 2.0f * (qxx +  qzz);
                    R[1][2] = 2.0f * (qyz + qwx);

                    R[2][0] = 2.0f * (qxz + qwy);
                    R[2][1] = 2.0f * (qyz - qwx);
                    R[2][2] = 1.0f - 2.0f * (qxx +  qyy);
                    return R;
                  };

                  glm::mat3 Rp = quaternionToMat3(tbnq);
                  tpDebug() << "Idx==" << idx << " R1=(" << R[0][0] << " " << R[1][0] << " " << R[2][0] << ")";
                  tpDebug() << "Idx==" << idx << " R2=(" << R[0][1] << " " << R[1][1] << " " << R[2][1] << ")";
                  tpDebug() << "Idx==" << idx << " R3=(" << R[0][2] << " " << R[1][2] << " " << R[2][2] << ")";
                  tpDebug() << "Idx==" << idx << " Q1=(" << Rp[0][0] << " " << Rp[1][0] << " " << Rp[2][0] << ")";
                  tpDebug() << "Idx==" << idx << " Q2=(" << Rp[0][1] << " " << Rp[1][1] << " " << Rp[2][1] << ")";
                  tpDebug() << "Idx==" << idx << " Q3=(" << Rp[0][2] << " " << Rp[1][2] << " " << Rp[2][2] << ")";
                  tpDebug() << "Idx==" << idx << " n=(" << v.normal.x << " " << v.normal.y << " " << v.normal.z << ")";
                  tpDebug() << "Idx=" << idx << " n.t=" << glm::dot(v.normal, tangent.at(idx));
                }
#endif
              }
            }

            std::pair<GLenum, G3DMaterialShader::VertexBuffer*> p;
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
    TP_FUNCTION_TIME("Geometry3DPool::PoolDetails_lt::checkUpdateVertexBufferTextures");

    if(updateVertexBufferTextures)
    {
      updateVertexBufferTextures=false;
      size_t mMax = tpMin(processedGeometry.size(), textureKeys.size());
      for(size_t m=0; m<mMax; m++)
      {
        auto& details = processedGeometry.at(m);
        const auto& textureDetails = textureKeys.at(m);

        details.    rgbaTextureID = texturePool->textureID(textureDetails.rgba   );
        details. normalsTextureID = texturePool->textureID(textureDetails.normals);
        details.   rmttrTextureID = texturePool->textureID(textureDetails.rmttr  );
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

  std::unique_ptr<TexturePool> ownedTexturePool;
  TexturePool* texturePool;

  std::unordered_map<tp_utils::StringID, PoolDetails_lt> pools;

  int keepHot{0};

  //################################################################################################
  Private(Geometry3DPool* q_, Map* map_, TexturePool* texturePool_):
    q(q_),
    m_map(map_),
    m_layer(nullptr),
    texturePool(texturePool_)
  {
    if(!texturePool)
    {
      ownedTexturePool = std::make_unique<TexturePool>(m_map);
      texturePool = ownedTexturePool.get();
    }

    invalidateBuffersCallback.connect(m_map->invalidateBuffersCallbacks);
    texturePoolChanged.connect(texturePool->changed);
    texturePoolChanged();
  }

  //################################################################################################
  Private(Geometry3DPool* q_, Layer* layer_, TexturePool* texturePool_):
    q(q_),
    m_map(nullptr),
    m_layer(layer_),
    texturePool(texturePool_)
  {
    if(!texturePool)
    {
      ownedTexturePool = std::make_unique<TexturePool>(m_layer);
      texturePool = ownedTexturePool.get();
    }

    invalidateBuffersCallback.connect(m_layer->invalidateBuffersCallbacks);
    texturePoolChanged.connect(texturePool->changed);
    texturePoolChanged();
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
#ifdef TP_MAPS_DEBUG
  void checkForDuplicateIDs()
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    static std::vector<GLuint> vaoIDs; vaoIDs.clear();
    static std::vector<GLuint> iboIDs; iboIDs.clear();
#endif
    static std::vector<GLuint> vboIDs; vboIDs.clear();

    for(auto& pool : pools)
    {
      for(const auto& processedGeometry : pool.second.processedGeometry)
      {
        for(const auto& mesh : processedGeometry.vertexBuffers)
        {
          assert(!tpContains(vaoIDs, mesh.second->vaoID)); vaoIDs.push_back(mesh.second->vaoID);
          assert(!tpContains(iboIDs, mesh.second->iboID)); iboIDs.push_back(mesh.second->iboID);
          assert(!tpContains(vboIDs, mesh.second->vboID)); vboIDs.push_back(mesh.second->vboID);
        }
      }
    }
  }
#  define CHECK_FOR_DUPLICATE_IDS() TP_CLEANUP([&]{checkForDuplicateIDs();})
#else
#  define CHECK_FOR_DUPLICATE_IDS() do{}while(false)
#endif

  //################################################################################################
  tp_utils::Callback<void()> texturePoolChanged = [&]
  {
    TP_FUNCTION_TIME("Geometry3DPool::Private::texturePoolChanged");
    CHECK_FOR_DUPLICATE_IDS();

    for(auto& i : pools)
      i.second.updateVertexBufferTextures=true;

    q->changed();
  };

  //################################################################################################
  tp_utils::Callback<void()> invalidateBuffersCallback = [&]
  {
    TP_FUNCTION_TIME("Geometry3DPool::Private::invalidateBuffersCallback");
    CHECK_FOR_DUPLICATE_IDS();

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
    TP_FUNCTION_TIME("Geometry3DPool::Private::unsubscribeTextures");
    CHECK_FOR_DUPLICATE_IDS();

    if(texturePool)
      for(const auto& key : textureSubscriptions)
        texturePool->unsubscribe(key);

    textureSubscriptions.clear();
  }
};

#ifdef TP_MAPS_DEBUG
#  undef CHECK_FOR_DUPLICATE_IDS
#  define CHECK_FOR_DUPLICATE_IDS() TP_CLEANUP([&]{d->checkForDuplicateIDs();})
#endif

//##################################################################################################
Geometry3DPool::Geometry3DPool(Map* map, TexturePool* texturePool):
  d(new Private(this, map, texturePool))
{

}

//##################################################################################################
Geometry3DPool::Geometry3DPool(Layer* layer, TexturePool* texturePool):
  d(new Private(this, layer, texturePool))
{

}

//##################################################################################################
Geometry3DPool::~Geometry3DPool()
{
  delete d;
}

//##################################################################################################
TexturePool* Geometry3DPool::texturePool() const
{
  return d->texturePool;
}

//##################################################################################################
void Geometry3DPool::incrementKeepHot(bool keepHot)
{
  TP_FUNCTION_TIME("Geometry3DPool::incrementKeepHot");
  CHECK_FOR_DUPLICATE_IDS();

  d->texturePool->incrementKeepHot(keepHot);

  d->keepHot += keepHot?1:-1;
  if(d->keepHot==0)
  {
    for(auto i = d->pools.begin(); i!=d->pools.end();)
    {
      if(!i->second.count)
      {
        i->second.deleteVertexBuffers();
        d->unsubscribeTextures(i->second.textureSubscriptions);
        i = d->pools.erase(i);
      }
      else
        ++i;
    }
  }
}

//##################################################################################################
void Geometry3DPool::subscribe(const tp_utils::StringID& name,
                               const std::function<std::vector<tp_math_utils::Geometry3D>()>& getGeometry,
                               bool overwrite,
                               bool isOnlyMaterial)
{
  TP_FUNCTION_TIME("Geometry3DPool::subscribe");
  CHECK_FOR_DUPLICATE_IDS();

  auto& details = d->pools[name];
  details.count++;
  if(details.isNew || overwrite || details.overwrite)
  {
    details.isNew = false;
    bool tileTextures=false;

    details.overwrite = false;
    details.geometry = getGeometry();
    details.updateVertexBuffer = true;
    details.isOnlyMaterial = isOnlyMaterial;
    d->unsubscribeTextures(details.textureSubscriptions);

    std::unordered_set<TexturePoolKey> textureSubscriptions;
    details.textureKeys.resize(details.geometry.size());
    for(size_t i=0; i<details.geometry.size(); i++)
    {
      const auto& material = details.geometry.at(i).material;
      auto& textureKeys = details.textureKeys.at(i);

      tileTextures = material.tileTextures;

      const auto& rgba    = material.                 rgbaTexture;
      const auto& rgb     = material.               albedoTexture;
      const auto& a       = material.                alphaTexture;
      const auto& normals = material.              normalsTexture;
      const auto& rmttr   = material.                rmttrTexture;
      const auto& r       = material.            roughnessTexture;
      const auto& m       = material.            metalnessTexture;
      const auto& t       = material.         transmissionTexture;
      const auto& tr      = material.transmissionRoughnessTexture;

      if(rgba.isValid()) textureKeys.rgba   = TexturePoolKey(rgba   , rgba   , rgba   ,  rgba, 0, 1, 2, 3, TPPixel(uint8_t(material. albedo.x*255.0f), uint8_t(material. albedo.y*255.0f), uint8_t(material.albedo.z*255.0f)    , uint8_t(material.alpha*255.0f)                ), NChannels::RGBA);
      else               textureKeys.rgba   = TexturePoolKey(rgb    , rgb    , rgb    ,     a, 0, 1, 2, 0, TPPixel(uint8_t(material. albedo.x*255.0f), uint8_t(material. albedo.y*255.0f), uint8_t(material.albedo.z*255.0f)    , uint8_t(material.alpha*255.0f)                ), NChannels::RGBA);
      textureKeys.normals                   = TexturePoolKey(normals, normals, normals,    {}, 0, 1, 2, 0, TPPixel(128                               , 128                               , 255                                  , 255                                           ), NChannels::RGB );
      if(rmttr.isValid())textureKeys.rmttr  = TexturePoolKey(rmttr  , rmttr  , rmttr  , rmttr, 0, 1, 2, 3, TPPixel(uint8_t(material.roughness*255.0f), uint8_t(material.metalness*255.0f), uint8_t(material.transmission*255.0f), uint8_t(material.transmissionRoughness*255.0f)), NChannels::RGBA);
      else               textureKeys.rmttr  = TexturePoolKey(r      ,  m     ,   t    ,    tr, 0, 0, 0, 0, TPPixel(uint8_t(material.roughness*255.0f), uint8_t(material.metalness*255.0f), uint8_t(material.transmission*255.0f), uint8_t(material.transmissionRoughness*255.0f)), NChannels::RGBA);

      textureSubscriptions.insert(textureKeys.rgba   );
      textureSubscriptions.insert(textureKeys.normals);
      textureSubscriptions.insert(textureKeys.rmttr  );
    }

    details.textureSubscriptions.swap(textureSubscriptions);
    for(const auto& key : details.textureSubscriptions)
    {
      d->texturePool->subscribe(key, false);
      d->texturePool->setTextureWrapS(key, tileTextures?GL_REPEAT:GL_CLAMP_TO_EDGE);
      d->texturePool->setTextureWrapT(key, tileTextures?GL_REPEAT:GL_CLAMP_TO_EDGE);
    }

    details.updateVertexBufferTextures=true;
  }
  changed();
}

//##################################################################################################
void Geometry3DPool::unsubscribe(const tp_utils::StringID& name)
{
  TP_FUNCTION_TIME("Geometry3DPool::unsubscribe");
  CHECK_FOR_DUPLICATE_IDS();

  auto i = d->pools.find(name);
  i->second.count--;
  if(!i->second.count && d->keepHot==0)
  {
    i->second.deleteVertexBuffers();
    d->unsubscribeTextures(i->second.textureSubscriptions);
    d->pools.erase(i);
  }
}

//##################################################################################################
void Geometry3DPool::invalidate(const tp_utils::StringID& name)
{
  CHECK_FOR_DUPLICATE_IDS();
  if(auto i = d->pools.find(name); i != d->pools.end())
    i->second.overwrite = true;
}

//##################################################################################################
void Geometry3DPool::viewProcessedGeometry(const tp_utils::StringID& name,
                                           Geometry3DShader* shader,
                                           const tp_math_utils::AlternativeMaterials& alternativeMaterials,
                                           const std::vector<glm::mat3>& uvMatrices,
                                           const ProcessedGeometryCallback& closure)
{
  TP_FUNCTION_TIME("Geometry3DPool::viewProcessedGeometry");
  CHECK_FOR_DUPLICATE_IDS();

  d->map()->makeCurrent();

  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  i->second.checkUpdateVertexBuffer(shader, d->map());
  i->second.checkUpdateVertexBufferTextures(d->texturePool);

  if(!i->second.isOnlyMaterial)
  {
    for(size_t c=0; c<i->second.processedGeometry.size(); c++)
    {
      auto& mesh = i->second.processedGeometry.at(c);

      mesh.uvMatrix = (c<uvMatrices.size())?uvMatrices.at(c):glm::mat3(1.0f);

      mesh.alternativeMaterial = &mesh;
      auto itr = alternativeMaterials.find(mesh.material.name);
      if(itr != alternativeMaterials.end())
      {
        viewProcessedGeometry(itr->second, shader, {}, {}, [&](const auto& geometry)
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
                                  const tp_math_utils::GeometryCallback& closure) const
{
  CHECK_FOR_DUPLICATE_IDS();

  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  closure(i->second.geometry);
}

//##################################################################################################
void Geometry3DPool::viewGeometry(const tp_utils::StringID& name,
                                  const tp_math_utils::AlternativeMaterials& alternativeMaterials,
                                  const tp_math_utils::GeometryMaterialCallback& closure) const
{
  CHECK_FOR_DUPLICATE_IDS();
  auto i = d->pools.find(name);
  if(i==d->pools.end())
    return;

  std::vector<tp_math_utils::Material> materials;
  materials.reserve(i->second.geometry.size());

  for(const auto& mesh : i->second.geometry)
  {
    auto itr = alternativeMaterials.find(mesh.material.name);
    if(itr != alternativeMaterials.end())
    {
      auto j = d->pools.find(itr->second);
      if(j!=d->pools.end() && !j->second.geometry.empty())
      {
        materials.push_back(j->second.geometry.front().material);
        continue;
      }
    }

    materials.push_back(mesh.material);
  }

  closure(i->second.geometry, materials);
}

//##################################################################################################
tp_math_utils::FindGeometry Geometry3DPool::findGeometryFunctor() const
{
  return [&](const auto& objectId, const auto& closure)
  {
    viewGeometry(objectId, closure);
  };
}

}
