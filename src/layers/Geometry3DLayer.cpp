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
};
}
//##################################################################################################
struct Geometry3DLayer::Private
{
  TP_NONCOPYABLE(Private);
  Geometry3DLayer* q;

  std::vector<Geometry3D> geometry;
  MaterialShader::Light light;
  Texture* texture;
  ShaderType shaderType{ShaderType::Material};

  glm::mat4 objectMatrix{1.0f};

  //Processed geometry ready for rendering
  std::vector<GeometryDetails_lt> processedGeometry;
  GLuint textureID{0};
  bool updateVertexBuffer{true};
  bool bindBeforeRender{true};

  //################################################################################################
  Private(Geometry3DLayer* q_, Texture* texture_):
    q(q_),
    texture(texture_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(textureID)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(textureID);
    }

    delete texture;
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

          std::vector<GLuint> indexes;
          std::vector<MaterialShader::Vertex> verts;
          for(size_t n=0; n<part.indexes.size(); n++)
          {
            auto idx = part.indexes.at(n);
            if(size_t(idx)<shape.geometry.verts.size())
            {
              const auto& v = shape.geometry.verts.at(size_t(idx));
              indexes.push_back(GLuint(n));
              verts.push_back(MaterialShader::Vertex(v.vert, v.normal, v.texture));
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
Geometry3DLayer::Geometry3DLayer(Texture* texture):
  d(new Private(this, texture))
{
  if(texture)
  {
    texture->setImageChangedCallback([this]()
    {
      d->bindBeforeRender = true;
      update();
    });
  }
}

//##################################################################################################
Geometry3DLayer::~Geometry3DLayer()
{
  delete d;
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


  //-- MaterialShader ------------------------------------------------------------------------------
  if(d->shaderType == ShaderType::Material)
  {
    render(static_cast<MaterialShader*>(nullptr), [&](auto shader)
    {
      shader->use();
      auto m = map()->controller()->matrices(coordinateSystem());
      shader->setMatrix(d->objectMatrix, m.v, m.p);
      shader->setCameraRay(m.cameraOriginNear, m.cameraOriginFar);
      shader->setLight(d->light);
    }, [&](auto, auto)
    {

    }, [&](auto shader, auto first, auto second, auto pickingID)
    {
      shader->drawPicking(first, second, pickingID);
    }, [&](auto shader, const auto& details)
    {
      shader->setMaterial(details.material);
    }, [&](auto shader, auto first, auto second)
    {
      shader->draw(first, second);
    },
    renderInfo);
  }


  //-- ImageShader ---------------------------------------------------------------------------------
  else if(d->shaderType == ShaderType::Image)
  {
    if(!d->texture->imageReady())
      return;

    if(d->bindBeforeRender)
    {
      d->bindBeforeRender=false;
      map()->deleteTexture(d->textureID);
      d->textureID = d->texture->bindTexture();
      d->updateVertexBuffer=true;
    }

    if(!d->textureID)
      return;

    render(static_cast<ImageShader*>(nullptr), [&](auto shader)
    {
      shader->use();
      auto m = map()->controller()->matrices(coordinateSystem());
      shader->setMatrix(m.vp * d->objectMatrix);
      shader->setTexture(d->textureID);
    }, [&](auto, auto)
    {

    }, [&](auto shader, auto first, auto second, auto pickingID)
    {
      shader->drawPicking(first, second, pickingID);
    }, [&](auto, const auto&)
    {

    }, [&](auto shader, auto first, auto second)
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
  d->updateVertexBuffer=true;
  d->textureID = 0;
  d->bindBeforeRender = true;
}

}
