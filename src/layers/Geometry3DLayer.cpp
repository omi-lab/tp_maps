#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Texture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"

#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/shaders/ImageShader.h"

#include "tp_utils/DebugUtils.h"

#include "tp_triangulation/Triangulation.h"

#include "glm/glm.hpp"

namespace tp_maps
{
namespace
{
struct GeometryDetails_lt
{
  std::vector<std::pair<GLenum, MaterialShader::VertexBuffer*>> vertexBuffers;
  MaterialShader::Material material;

  GLuint ambientTextureID{0};
  GLuint diffuseTextureID{0};
  GLuint specularTextureID{0};
  GLuint alphaTextureID{0};
  GLuint bumpTextureID{0};
};
}
//##################################################################################################
struct Geometry3DLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Geometry3DLayer::Private");
  TP_NONCOPYABLE(Private);

  Geometry3DLayer* q;

  //-- Input data ----------------------------------------------------------------------------------
  std::vector<Geometry3D> geometry;
  MaterialShader::Light light;
  std::unordered_map<tp_utils::StringID, Texture*> textures;
  ShaderType shaderType{ShaderType::Material};
  glm::mat4 objectMatrix{1.0f};
  std::unique_ptr<BasicTexture> emptyTexture;

  //-- Preprocessed data ---------------------------------------------------------------------------
  std::vector<GeometryDetails_lt> processedGeometry;
  std::unordered_map<tp_utils::StringID, GLuint> textureIDs;
  GLuint emptyTextureID{0};

  bool updateVertexBuffer{true};
  bool bindBeforeRender{true};

  //################################################################################################
  Private(Geometry3DLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(!textureIDs.empty())
    {
      q->map()->makeCurrent();
      for(auto i : textureIDs)
        if(i.second)
          q->map()->deleteTexture(i.second);
    }

    for(auto i : textures)
      delete i.second;
    textures.clear();

    deleteVertexBuffers();
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    for(const auto& details : processedGeometry)
      for(const auto& buffer : details.vertexBuffers)
        delete buffer.second;

    processedGeometry.clear();
  }

  //################################################################################################
  void checkUpdateVertexBuffer(Geometry3DShader* shader)
  {
    if(updateVertexBuffer)
    {
      deleteVertexBuffers();
      updateVertexBuffer=false;

      for(const auto& shape : geometry)
      {
        for(const auto& part : shape.geometry.indexes)
        {
          GeometryDetails_lt details;
          details.material = shape.material;

          auto selectTexture = [&](const tp_utils::StringID& name, GLuint& textureID, glm::vec3& color)
          {
            if(auto i=textureIDs.find(name); i!=textureIDs.end())
            {
              textureID = i->second;
              color = {0.0f, 0.0f, 0.0f};
            }
            else
            {
              textureID = emptyTextureID;
            }
          };

          glm::vec3 tmpNormal{0.0f, 0.0f, 1.0f};
          selectTexture(details.material.ambientTexture, details.ambientTextureID, details.material.ambient);
          selectTexture(details.material.diffuseTexture, details.diffuseTextureID, details.material.diffuse);
          selectTexture(details.material.specularTexture, details.specularTextureID, details.material.specular);
          details.alphaTextureID = emptyTextureID;
          selectTexture(details.material.bumpTexture, details.bumpTextureID, tmpNormal);

          std::vector<GLuint> indexes;
          std::vector<MaterialShader::Vertex> verts;
          for(size_t n=0; n<part.indexes.size(); n++)
          {
            auto idx = part.indexes.at(n);
            if(size_t(idx)<shape.geometry.verts.size())
            {
              const auto& v = shape.geometry.verts.at(size_t(idx));
              indexes.push_back(GLuint(n));
              verts.emplace_back(MaterialShader::Vertex(v.vert, v.normal, v.texture));
            }
          }

          std::pair<GLenum, MaterialShader::VertexBuffer*> p;
          p.first = GLenum(part.type);
          p.second = shader->generateVertexBuffer(q->map(), indexes, verts);
          details.vertexBuffers.push_back(p);

          processedGeometry.push_back(details);
        }
      }
    }
  }
};

//##################################################################################################
Geometry3DLayer::Geometry3DLayer():
  d(new Private(this))
{

}

//##################################################################################################
Geometry3DLayer::~Geometry3DLayer()
{
  delete d;
}

//##################################################################################################
void Geometry3DLayer::setTextures(const std::unordered_map<tp_utils::StringID, Texture*>& textures)
{
  for(auto i : d->textures)
    delete i.second;
  d->textures = textures;

  for(auto i : d->textures)
  {
    i.second->setImageChangedCallback([this]()
    {
      d->bindBeforeRender = true;
      update();
    });
  }
  d->bindBeforeRender = true;
}

//##################################################################################################
const std::vector<Geometry3D>& Geometry3DLayer::geometry()const
{
  return d->geometry;
}

//##################################################################################################
void Geometry3DLayer::setGeometry(const std::vector<Geometry3D>& geometry)
{
  d->geometry = geometry;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
const glm::mat4& Geometry3DLayer::objectMatrix()const
{
  return d->objectMatrix;
}

//##################################################################################################
void Geometry3DLayer::setObjectMatrix(const glm::mat4& objectMatrix)
{
  d->objectMatrix = objectMatrix;
  update();
}

//##################################################################################################
void Geometry3DLayer::setLight(const MaterialShader::Light& light)
{
  d->light = light;
  update();
}

//##################################################################################################
void Geometry3DLayer::setMaterial(const MaterialShader::Material& material)
{
  for(auto& g : d->geometry)
    g.material = material;

  for(auto& g : d->processedGeometry)
    g.material = material;

  update();
}

//##################################################################################################
void Geometry3DLayer::setShaderType(ShaderType shaderType)
{
  d->shaderType = shaderType;
  update();
}

//##################################################################################################
Geometry3DLayer::ShaderType Geometry3DLayer::shaderType() const
{
  return d->shaderType;
}

//##################################################################################################
void Geometry3DLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  //== Bind Textures ===============================================================================
  if(d->bindBeforeRender)
  {
    d->bindBeforeRender=false;
    d->updateVertexBuffer=true;

    for(auto i : d->textureIDs)
      if(i.second)
        map()->deleteTexture(i.second);
    d->textureIDs.clear();

    if(d->emptyTextureID)
      map()->deleteTexture(d->emptyTextureID);
    d->emptyTextureID = 0;

    for(auto i : d->textures)
    {
      if(!i.second->imageReady())
        continue;

      auto textureID = i.second->bindTexture();
      if(!textureID)
        continue;

      d->textureIDs[i.first] = textureID;
    }

    if(!d->emptyTexture)
    {
      TPPixel white{0, 0, 0, 255};
      TextureData textureData;
      textureData.w = 1;
      textureData.h = 1;
      textureData.data = &white;
      d->emptyTexture = std::make_unique<BasicTexture>(map(), textureData);
    }

    d->emptyTextureID = d->emptyTexture->bindTexture();
  }

  //== Common ======================================================================================
  auto render = [&](auto s,
      const auto& use,
      const auto& setMaterialPicking,
      const auto& drawPicking,
      const auto& setMaterial,
      const auto& draw,
      RenderInfo& renderInfo)
  {
    auto shader = map()->getShader<typename std::remove_pointer<decltype(s)>::type>();
    if(shader->error())
      return;

    d->checkUpdateVertexBuffer(shader);

    use(shader);

    if(renderInfo.pass==RenderPass::Picking)
    {
      size_t iMax = d->processedGeometry.size();
      for(size_t i=0; i<iMax; i++)
      {
        const GeometryDetails_lt& details = d->processedGeometry.at(i);
        auto pickingID = renderInfo.pickingIDMat(PickingDetails(i, [](const PickingResult& r)
        {
          return new GeometryPickingResult(r.pickingType, r.details, r.renderInfo);
        }));

        setMaterialPicking(shader, details);
        for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
          drawPicking(shader, buff.first, buff.second, pickingID);
      }
    }
    else
    {
      for(const auto& details : d->processedGeometry)
      {
        setMaterial(shader, details);
        for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
          draw(shader, buff.first, buff.second);
      }
    }
  };

  //== MaterialShader ==============================================================================
  if(d->shaderType == ShaderType::Material)
  {

    render(static_cast<MaterialShader*>(nullptr), [&](auto shader) //-- use ------------------------
    {
      shader->use();
      auto m = map()->controller()->matrices(coordinateSystem());
      shader->setMatrix(d->objectMatrix, m.v, m.p);
      shader->setCameraRay(m.cameraOriginNear, m.cameraOriginFar);
      shader->setLight(d->light);
    },
    [&](auto, const auto&) //-- setMaterialPicking -------------------------------------------------
    {

    },
    [&](auto shader, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setMaterial(details.material);
      shader->setTextures(details.ambientTextureID,
                          details.diffuseTextureID,
                          details.specularTextureID,
                          details.alphaTextureID,
                          details.bumpTextureID);
    },
    [&](auto shader, auto first, auto second) //-- draw --------------------------------------------
    {
      shader->draw(first, second);
    },
    renderInfo);
  }

  //== ImageShader =================================================================================
  else if(d->shaderType == ShaderType::Image)
  {
    render(static_cast<ImageShader*>(nullptr), [&](auto shader) //-- use ---------------------------
    {
      shader->use();
      auto m = map()->controller()->matrices(coordinateSystem());
      shader->setMatrix(m.vp * d->objectMatrix);
    },
    [&](auto shader, const auto& details) //-- setMaterialPicking ----------------------------------
    {
      shader->setTexture(details.ambientTextureID);
    },
    [&](auto shader, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setTexture(details.ambientTextureID);
    },
    [&](auto shader, auto first, auto second) //-- draw --------------------------------------------
    {
      shader->draw(first, second, {1.0f, 1.0f, 1.0f, 1.0f});
    },
    renderInfo);
  }
}

//##################################################################################################
void Geometry3DLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer = true;
  d->bindBeforeRender = true;
  d->textureIDs.clear();
}

}
