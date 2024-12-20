#include "tp_maps/layers/GeometryLayer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/G3DFlatColorShader.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"

#include "tp_triangulation/Triangulation.h"

#include "tp_math_utils/materials/OpenGLMaterial.h"

namespace tp_maps
{

namespace
{
struct GeometryDetails_lt
{
  std::vector<std::pair<GLenum, Geometry3DShader::VertexBuffer*>> vertexBuffers;
  glm::vec4 color;
//  tp_math_utils::OpenGLMaterial material;
};
}
//##################################################################################################
struct GeometryLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Geometry3DLayer::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  std::vector<tp_math_utils::Geometry> geometry;
  bool drawBackFaces{false};

  //Processed geometry ready for rendering
  std::vector<GeometryDetails_lt> processedGeometry;
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(Q* q_):
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
bool GeometryLayer::drawBackFaces() const
{
  return d->drawBackFaces;
}

//##################################################################################################
void GeometryLayer::setDrawBackFaces(bool drawBackFaces)
{
  d->drawBackFaces = drawBackFaces;
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

  auto shader = map()->getShader<G3DFlatColorShader>();
  if(shader->error())
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();
    d->updateVertexBuffer=false;

    for(const auto& shape : d->geometry)
    {
      GeometryDetails_lt details;
      shape.material.viewOpenGL([&](const auto& m){details.color = m.rgba();});

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
          std::vector<Geometry3DShader::Vertex> verts;

          indexes.reserve(d->drawBackFaces?c.vertices.size()*2:c.vertices.size());
          verts.reserve(c.vertices.size());

          for(size_t n=0; n<c.vertices.size(); n++)
          {
            const auto& v = c.vertices.at(n);
            indexes.push_back(GLuint(n));            
            glm::vec4 vv = shape.transform * glm::vec4(v.x, v.y, 0.0f, 1.0f);

            //                                        vertex                     tbnq                  texture
            verts.push_back(Geometry3DShader::Vertex(glm::vec3(vv) / vv.w, {0.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}));
          }

          if(indexes.size()>0 && d->drawBackFaces)
            for(size_t i=indexes.size()-1; i<indexes.size(); i--)
              indexes.push_back(indexes.at(i));


          std::pair<GLenum, Geometry3DShader::VertexBuffer*> p;
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
    shader->setMatrix(m.vp*modelToWorldMatrix());
  }

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
      for(const std::pair<GLenum, Geometry3DShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->draw(buff.first, buff.second, pickingID);
    }
  }
  else
  {
    for(const auto& details : d->processedGeometry)
    {
      // shader->setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
      for(const std::pair<GLenum, Geometry3DShader::VertexBuffer*>& buff : details.vertexBuffers)
        shader->draw(buff.first, buff.second, details.color);
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
