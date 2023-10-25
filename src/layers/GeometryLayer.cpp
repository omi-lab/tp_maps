#include "tp_maps/layers/GeometryLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/G3DMaterialShader.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"

#include "tp_triangulation/Triangulation.h"

namespace tp_maps
{

namespace
{
struct GeometryDetails_lt
{
  std::vector<std::pair<GLenum, G3DMaterialShader::VertexBuffer*>> vertexBuffers;
  tp_math_utils::Material material;
};
}
//##################################################################################################
struct GeometryLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Geometry3DLayer::Private");
  TP_NONCOPYABLE(Private);

  GeometryLayer* q;

  std::vector<tp_math_utils::Geometry> geometry;

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
const std::vector<tp_math_utils::Geometry>& GeometryLayer::geometry() const
{
  return d->geometry;
}

//##################################################################################################
void GeometryLayer::setGeometry(const std::vector<tp_math_utils::Geometry>& geometry)
{
  d->geometry = geometry;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void GeometryLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass().type &&
     renderInfo.pass != RenderPass::Transparency &&
     renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = map()->getShader<G3DMaterialShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const auto& shape : d->geometry)
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
          std::vector<GLuint> indexes;
          std::vector<G3DMaterialShader::Vertex> verts;
          verts.reserve(c.vertices.size());
          for(size_t n=0; n<c.vertices.size(); n++)
          {
            const auto& v = c.vertices.at(n);
            indexes.push_back(GLuint(n));            
            glm::vec4 vv = shape.transform * glm::vec4(v.x, v.y, 0.0f, 1.0f);
            verts.push_back(Geometry3DShader::Vertex(glm::vec3(vv) / vv.w, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}));
          }

          std::pair<GLenum, G3DMaterialShader::VertexBuffer*> p;
          p.first = GLenum(i.first);
          p.second = shader->generateVertexBuffer(map(), indexes, verts);
          details.vertexBuffers.push_back(p);
        }
      }

      d->processedGeometry.push_back(details);
    }
  }

  shader->use(renderInfo.shaderType());

  {
    auto m = map()->controller()->matrices(coordinateSystem());
    shader->setMatrix(modelToWorldMatrix(), m.v, m.p);
  }
  shader->setLights(map()->lights(), map()->lightBuffers());
  shader->setLightOffsets(map()->renderedLightLevels());

  if(renderInfo.pass==RenderPass::Picking)
  {
    size_t iMax = d->processedGeometry.size();
    for(size_t i=0; i<iMax; i++)
    {
      const GeometryDetails_lt& details = d->processedGeometry.at(i);
      auto pickingID = renderInfo.pickingIDMat(PickingDetails(i, [&](const PickingResult& r)
      {
        return new GeometryPickingResult(r.pickingType, r.details, r.renderInfo, this);
      }));
      for(const std::pair<GLenum, G3DMaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->drawPicking(buff.first, buff.second, pickingID);
    }
  }
  else
  {
    for(const auto& details : d->processedGeometry)
    {
      shader->setMaterial(details.material);
      shader->setBlankTextures();
      shader->setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
      for(const std::pair<GLenum, G3DMaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->draw(buff.first, buff.second);
    }
  }
}

//##################################################################################################
void GeometryLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  Layer::invalidateBuffers();
}

}
