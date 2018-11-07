#include "tp_maps/layers/GeometryLayer.h"
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
struct GeometryLayer::Private
{
  TP_NONCOPYABLE(Private);
  GeometryLayer* q;

  std::vector<Geometry> geometry;
  MaterialShader::Light light;

  //Processed geometry ready for rendering
  std::vector<GeometryDetails_lt> processedGeometry;
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(GeometryLayer* q_):
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
GeometryLayer::GeometryLayer():
  d(new Private(this))
{

}

//##################################################################################################
GeometryLayer::~GeometryLayer()
{
  delete d;
}

//##################################################################################################
const std::vector<Geometry>& GeometryLayer::geometry()const
{
  return d->geometry;
}

//##################################################################################################
void GeometryLayer::setGeometry(const std::vector<Geometry>& geometry)
{
  d->geometry = geometry;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GeometryLayer::setLight(const MaterialShader::Light& light)
{
  d->light = light;
  update();
}

//##################################################################################################
void GeometryLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != RenderPass::Normal && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<MaterialShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const Geometry& shape : d->geometry)
    {
      GeometryDetails_lt details;
      details.material = shape.material;

      std::vector<tp_triangulation::Polygon> srcData;
      tp_triangulation::Polygon polygon;
      tp_triangulation::Contour contour;
      contour.vertices = shape.geometry;
      polygon.contours.push_back(contour);
      srcData.push_back(polygon);

      std::map<int, std::vector<tp_triangulation::Contour>> resultVerts;
      tp_triangulation::triangulate(srcData, GL_TRIANGLE_FAN, GL_TRIANGLE_STRIP, GL_TRIANGLES, resultVerts);

      for(const auto& i : resultVerts)
      {
        for(const tp_triangulation::Contour& c : i.second)
        {
          std::vector<GLushort> indexes;
          std::vector<MaterialShader::Vertex> verts;
          for(size_t n=0; n<c.vertices.size(); n++)
          {
            const glm::vec3& v = c.vertices.at(n);
            indexes.push_back(GLushort(n));
            verts.push_back(MaterialShader::Vertex(v, {0.0f, 0.0f, 1.0f}));
          }

          std::pair<GLenum, MaterialShader::VertexBuffer*> p;
          p.first = GLenum(i.first);
          p.second = shader->generateVertexBuffer(map(), indexes, verts);
          details.vertexBuffers.push_back(p);
        }
      }

      d->processedGeometry.push_back(details);
    }
  }

  shader->use();
  shader->setMatrix(map()->controller()->matrix(coordinateSystem()));
  shader->setLight(d->light);

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
void GeometryLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
