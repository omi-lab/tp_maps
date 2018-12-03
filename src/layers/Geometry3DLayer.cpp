#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Texture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"

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

  glm::mat4 objectMatrix{1.0f};

  //Processed geometry ready for rendering
  std::vector<GeometryDetails_lt> processedGeometry;
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(Geometry3DLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
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

//################################################################################################
const glm::mat4& Geometry3DLayer::objectMatrix()const
{
  return d->objectMatrix;
}

//################################################################################################
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
void Geometry3DLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<MaterialShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const auto& shape : d->geometry)
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
            verts.push_back(MaterialShader::Vertex(v.vert, v.normal));
          }
        }

        std::pair<GLenum, MaterialShader::VertexBuffer*> p;
        p.first = GLenum(part.type);
        p.second = shader->generateVertexBuffer(map(), indexes, verts);
        details.vertexBuffers.push_back(p);

        d->processedGeometry.push_back(details);
      }
    }
  }

  {
    shader->use();
    auto m = map()->controller()->matrices(coordinateSystem());
    shader->setMatrix(d->objectMatrix, m.v, m.p);
    shader->setCameraRay(m.cameraOriginNear, m.cameraOriginFar);
    shader->setLight(d->light);
  }

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
      for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->drawPicking(buff.first, buff.second, pickingID);
    }
  }
  else
  {
    for(const auto& details : d->processedGeometry)
    {
      shader->setMaterial(details.material);
      for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->draw(buff.first, buff.second);
    }
  }
}

//##################################################################################################
void Geometry3DLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
