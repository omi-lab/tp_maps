#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Map.h"
#include "tp_maps/TexturePool.h"
#include "tp_maps/TexturePoolKey.h"

#include "tp_utils/TimeUtils.h"

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
  static float axisDot(const glm::fquat& q1, const glm::fquat& q2)
  {
    return q1.x*q2.x + q1.y*q2.y + q1.z*q2.z;
  }

  //################################################################################################
  static G3DMaterialShader::Vertex mixVertex(G3DMaterialShader::Vertex vert1, const G3DMaterialShader::Vertex& vert2, bool checkSign=false)
  {
    float alpha;
    if(!checkSign || axisDot(vert1.tbnq, vert2.tbnq) < 0.f)
    {
      vert1.tbnq = -vert1.tbnq;
      alpha = vert1.tbnq.w/(vert1.tbnq.w-vert2.tbnq.w);
    }
    else
      // same sign - take the average
      alpha = 0.5f;

    G3DMaterialShader::Vertex vert;
    vert.position = alpha*vert2.position + (1.f-alpha)*vert1.position;
    vert.position = alpha*vert2.position + (1.f-alpha)*vert1.position;
    vert.tbnq = alpha*vert2.tbnq + (1.f-alpha)*vert1.tbnq;
    vert.tbnq = alpha*vert2.tbnq + (1.f-alpha)*vert1.tbnq;
    vert.texture = alpha*vert2.texture + (1.f-alpha)*vert1.texture;
    vert.texture = alpha*vert2.texture + (1.f-alpha)*vert1.texture;
    return vert;
  }

  //################################################################################################
  static void addTriangle(std::vector<G3DMaterialShader::Vertex>& verts,
                          std::vector<GLuint>& indexes,
                          const G3DMaterialShader::Vertex& vert1,
                          const G3DMaterialShader::Vertex& vert2,
                          const G3DMaterialShader::Vertex& vert3,
                          bool checkSign=false)
  {
    verts.push_back(vert1);
    indexes.push_back(GLuint(indexes.size()));

    verts.push_back(vert2);
    auto& v2 = verts.back();
    indexes.push_back(GLuint(indexes.size()));
    if(checkSign && axisDot(vert1.tbnq, v2.tbnq) < 0.f)
      v2.tbnq = -v2.tbnq;

    verts.push_back(vert3);
    //auto& v3 = verts.back();
    indexes.push_back(GLuint(indexes.size()));
  }
  
  //################################################################################################
  static void overwriteExistingVertex(std::vector<G3DMaterialShader::Vertex>& verts,
                                      int i,
                                      const G3DMaterialShader::Vertex& v,
                                      int iref = -1)
  {
    verts[i] = v;
    if(0>iref || axisDot(v.tbnq, verts[iref].tbnq) < 0.f)
      verts[i].tbnq = -v.tbnq;
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

        ProcessedGeometry3D details;

        for(const auto& part : shape.indexes)
        {
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

                // convention for quaternion sign is that the w component should be positive
                if(tbnq.w < 0.f)
                  tbnq = -tbnq;

                verts.emplace_back(G3DMaterialShader::Vertex(v.vert, tbnq, v.texture));
              }
            }

            // check for triangles with inconsistent quaternion axis direction
            if(part.type == shape.triangles)
            {
              const size_t vsize = verts.size();
              for(size_t n=0; n<vsize; n+=3)
              {
                auto quaternionToRZ = [](const glm::quat& q)
                {
                  glm::vec3 RZ;
                  RZ[0] = 2.0f*(q.x*q.z + q.w*q.y);
                  RZ[1] = 2.0f*(q.y*q.z - q.w*q.x);
                  RZ[2] = 1.0f - 2.0f*(q.x*q.x +  q.y*q.y);
                  return RZ;
                };

                // we will check quaternion sign change when the normals are consistent
                glm::vec3 n1 = quaternionToRZ(verts[n].tbnq);
                glm::vec3 n2 = quaternionToRZ(verts[n+1].tbnq);
                glm::vec3 n3 = quaternionToRZ(verts[n+2].tbnq);
                const float normalConsistencyThres = 0.5f;
                if(glm::dot(n1,n2) > normalConsistencyThres && glm::dot(n1,n3) > normalConsistencyThres && glm::dot(n2,n3) > normalConsistencyThres)
                {
                  // the normals are consistent so the quaternions should be too. If they aren't it means that there is a sign change
                  // in the rotation angle - we will introduce new triangles to avoid the sign change
                  const auto v1 = verts[n];
                  const auto v2 = verts[n+1];
                  const auto v3 = verts[n+2];
                  const float dot12 = axisDot(v1.tbnq, v2.tbnq);
                  const float dot13 = axisDot(v1.tbnq, v3.tbnq);
                  const float dot23 = axisDot(v2.tbnq, v3.tbnq);

                  // check for case that vertex 1 is inconsistent with vertices 2,3
                  if(dot12 < 0.f && dot13 < 0.f && dot23 > 0.f)
                  {
                    // define two new vertices with zero rotation angle
                    G3DMaterialShader::Vertex v12 = mixVertex(v1, v2);
                    G3DMaterialShader::Vertex v13 = mixVertex(v1, v3);

                    // build two new triangles
                    addTriangle(verts, indexes, v12, v2, v3);
                    addTriangle(verts, indexes, v13, v12, v3);
                  
                    // overwrite two vertices of existing triangle
                    overwriteExistingVertex(verts, int(n+1), v12);
                    overwriteExistingVertex(verts, int(n+2), v13);
                  }
                  // check for case that vertex 2 is inconsistent with vertices 1,3
                  else if(dot12 < 0.f && dot13 > 0.f && dot23 < 0.f)
                  {
                    // define two new vertices with zero rotation angle
                    G3DMaterialShader::Vertex v23 = mixVertex(v2, v3);
                    G3DMaterialShader::Vertex v12 = mixVertex(v2, v1);

                    // build two new triangles
                    addTriangle(verts, indexes, v23, v3, v1);
                    addTriangle(verts, indexes, v12, v23, v1);

                    // overwrite two vertices of existing triangle
                    overwriteExistingVertex(verts, int(n+2), v23);
                    overwriteExistingVertex(verts, int(n  ), v12);
                  }
                  // check for case that vertex 3 is inconsistent with vertices 1,2
                  else if(dot12 > 0.f && dot13 < 0.f && dot23 < 0.f)
                  {
                    // define two new vertices with zero rotation angle
                    G3DMaterialShader::Vertex v13 = mixVertex(v3, v1);
                    G3DMaterialShader::Vertex v23 = mixVertex(v3, v2);

                    // build two new triangles
                    addTriangle(verts, indexes, v13, v1, v2);
                    addTriangle(verts, indexes, v23, v13, v2);

                    // overwrite two vertices of existing triangle
                    overwriteExistingVertex(verts, int(n  ), v13);
                    overwriteExistingVertex(verts, int(n+1), v23);
                  }
#if 0
                  // check for unhandled case - split into four triangles
                  else if(dot12 < 0.f || dot13 < 0.f || dot23 < 0.f)
                  {
                    G3DMaterialShader::Vertex v12 = mixVertex(v1, v2, true/*checkSign*/);
                    G3DMaterialShader::Vertex v13 = mixVertex(v1, v3, true/*checkSign*/);
                    G3DMaterialShader::Vertex v23 = mixVertex(v2, v3, true/*checkSign*/);

                    // build three new triangles
                    addTriangle(verts, indexes, v12, v2,  v23, true/*checkSign*/);
                    addTriangle(verts, indexes, v13, v23, v3,  true/*checkSign*/);
                    addTriangle(verts, indexes, v23, v13, v12, true/*checkSign*/);

                    // overwrite two vertices of existing triangle
                    overwriteExistingVertex(verts, n+1, v12, n/*iref*/);
                    overwriteExistingVertex(verts, n+2, v13, n/*iref*/);
                  }
#endif
                }
              }
            }

            std::pair<GLenum, G3DMaterialShader::VertexBuffer*> p;
            p.first = GLenum(part.type);
            p.second = shader->generateVertexBuffer(map, indexes, verts);
            details.vertexBuffers.push_back(p);
          }
        }

        processedGeometry.push_back(details);
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
