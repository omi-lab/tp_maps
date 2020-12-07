#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/PickingResult.h"

#include "tp_math_utils/Plane.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

namespace tp_maps
{

namespace
{
enum class Modify_lt
{
  None,
  RotateX,
  RotateY,
  RotateZ,
  TranslateX,
  TranslateY,
  TranslateZ
};
}

//##################################################################################################
struct GizmoLayer::Private
{
  Geometry3DLayer* rotateXGeometryLayer{nullptr};
  Geometry3DLayer* rotateYGeometryLayer{nullptr};
  Geometry3DLayer* rotateZGeometryLayer{nullptr};

  Geometry3DLayer* translateXGeometryLayer{nullptr};
  Geometry3DLayer* translateYGeometryLayer{nullptr};
  Geometry3DLayer* translateZGeometryLayer{nullptr};

  Modify_lt activeModification{Modify_lt::None};
  glm::ivec2 previousPos;

  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  float ringHeight{0.01f};
  float outerRadius{1.00f};
  float innerRadius{0.95f};
  float spikeRadius{0.90f};

  bool updateRotationGeometry{true};
  bool updateTranslationGeometry{true};

  //################################################################################################
  void generateRotationGeometry()
  {
    auto makeCircle = [&](std::vector<Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const glm::vec3& color)
    {
      auto& circle = geometry.emplace_back();

      circle.material.albedo = color;

      circle.geometry.triangleFan   = GL_TRIANGLE_FAN;
      circle.geometry.triangleStrip = GL_TRIANGLE_STRIP;
      circle.geometry.triangles     = GL_TRIANGLES;

      circle.geometry.indexes.resize(4);
      auto& top = circle.geometry.indexes.at(0);
      auto& btm = circle.geometry.indexes.at(1);
      auto& out = circle.geometry.indexes.at(2);
      auto& mid = circle.geometry.indexes.at(3);

      top.type = circle.geometry.triangleStrip;
      btm.type = circle.geometry.triangleStrip;
      out.type = circle.geometry.triangleStrip;
      mid.type = circle.geometry.triangleStrip;

      for(size_t a=0; a<=360; a+=6)
      {
        float x = std::sin(glm::radians(float(a)));
        float y = std::cos(glm::radians(float(a)));

        glm::vec2 v{x, y};

        tp_math_utils::Vertex3D vert;

        vert.vert = transform(glm::vec3(v*outerRadius, ringHeight));
        circle.geometry.verts.push_back(vert);
        vert.vert = transform(glm::vec3(v*outerRadius, -ringHeight));
        circle.geometry.verts.push_back(vert);

        auto iRad = (a%20)?innerRadius:spikeRadius;

        vert.vert = transform(glm::vec3(v*iRad, ringHeight));
        circle.geometry.verts.push_back(vert);
        vert.vert = transform(glm::vec3(v*iRad, -ringHeight));
        circle.geometry.verts.push_back(vert);

        int i = int(circle.geometry.verts.size());

        top.indexes.push_back(i-4);
        top.indexes.push_back(i-2);

        btm.indexes.push_back(i-1);
        btm.indexes.push_back(i-3);

        out.indexes.push_back(i-3);
        out.indexes.push_back(i-4);

        mid.indexes.push_back(i-2);
        mid.indexes.push_back(i-1);
      }

      circle.geometry.calculateFaceNormals();
    };

    std::vector<Geometry3D> rotateXGeometry;
    std::vector<Geometry3D> rotateYGeometry;
    std::vector<Geometry3D> rotateZGeometry;

    makeCircle(rotateXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, glm::vec3(1,0,0));
    makeCircle(rotateYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, glm::vec3(0,1,0));
    makeCircle(rotateZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, glm::vec3(0,0,1));

    rotateXGeometryLayer->setGeometry(rotateXGeometry);
    rotateYGeometryLayer->setGeometry(rotateYGeometry);
    rotateZGeometryLayer->setGeometry(rotateZGeometry);
  }

  //################################################################################################
  void generateTranslationGeometry()
  {
    auto makeArrow = [&](std::vector<Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const glm::vec3& color)
    {
      auto& arrow = geometry.emplace_back();

      arrow.material.albedo = color;

      arrow.geometry.triangleFan   = GL_TRIANGLE_FAN;
      arrow.geometry.triangleStrip = GL_TRIANGLE_STRIP;
      arrow.geometry.triangles     = GL_TRIANGLES;

      float stemRadius=0.05f;
      float coneRadius=0.1f;
      float stemStart=0.1f;
      float stemEnd=0.80f;
      float coneEnd=1.0f;

      tp_math_utils::Vertex3D vert;

      // Point
      vert.vert = transform(glm::vec3(0.0f, 0.0f, coneEnd));
      arrow.geometry.verts.push_back(vert);

      // Origin
      vert.vert = transform(glm::vec3(0.0f, 0.0f, stemStart));
      arrow.geometry.verts.push_back(vert);

      arrow.geometry.indexes.resize(1);
      auto& indexes = arrow.geometry.indexes.at(0);
      indexes.type = arrow.geometry.triangles;

      for(size_t a=0; a<=360; a+=10)
      {
        float x = std::sin(glm::radians(float(a)));
        float y = std::cos(glm::radians(float(a)));

        glm::vec2 v{x, y};

        vert.vert = transform(glm::vec3(v*stemRadius, stemStart));
        arrow.geometry.verts.push_back(vert);

        vert.vert = transform(glm::vec3(v*stemRadius, stemEnd));
        arrow.geometry.verts.push_back(vert);

        vert.vert = transform(glm::vec3(v*coneRadius, stemEnd));
        arrow.geometry.verts.push_back(vert);

        int i = int(arrow.geometry.verts.size());

        if(a==0)
          continue;

        indexes.indexes.push_back(1);
        indexes.indexes.push_back(i-6);
        indexes.indexes.push_back(i-3);


        indexes.indexes.push_back(i-3);
        indexes.indexes.push_back(i-6);
        indexes.indexes.push_back(i-2);

        indexes.indexes.push_back(i-5);
        indexes.indexes.push_back(i-2);
        indexes.indexes.push_back(i-6);


        indexes.indexes.push_back(i-2);
        indexes.indexes.push_back(i-5);
        indexes.indexes.push_back(i-1);

        indexes.indexes.push_back(i-4);
        indexes.indexes.push_back(i-1);
        indexes.indexes.push_back(i-5);


        indexes.indexes.push_back(0);
        indexes.indexes.push_back(i-1);
        indexes.indexes.push_back(i-4);
      }

      arrow.geometry.calculateFaceNormals();
    };

    std::vector<Geometry3D> translateXGeometry;
    std::vector<Geometry3D> translateYGeometry;
    std::vector<Geometry3D> translateZGeometry;

    makeArrow(translateXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, glm::vec3(1,0,0));
    makeArrow(translateYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, glm::vec3(0,1,0));
    makeArrow(translateZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, glm::vec3(0,0,1));

    translateXGeometryLayer->setGeometry(translateXGeometry);
    translateYGeometryLayer->setGeometry(translateYGeometry);
    translateZGeometryLayer->setGeometry(translateZGeometry);
  }
};

//##################################################################################################
GizmoLayer::GizmoLayer():
  d(new Private())
{
  d->rotateXGeometryLayer = new Geometry3DLayer(); addChildLayer(d->rotateXGeometryLayer);
  d->rotateYGeometryLayer = new Geometry3DLayer(); addChildLayer(d->rotateYGeometryLayer);
  d->rotateZGeometryLayer = new Geometry3DLayer(); addChildLayer(d->rotateZGeometryLayer);

  d->translateXGeometryLayer = new Geometry3DLayer(); addChildLayer(d->translateXGeometryLayer);
  d->translateYGeometryLayer = new Geometry3DLayer(); addChildLayer(d->translateYGeometryLayer);
  d->translateZGeometryLayer = new Geometry3DLayer(); addChildLayer(d->translateZGeometryLayer);
}

//##################################################################################################
GizmoLayer::~GizmoLayer()
{
  delete d;
}

//##################################################################################################
void GizmoLayer::setEnableRotation(bool x, bool y, bool z)
{
  d->rotateXGeometryLayer->setVisible(x);
  d->rotateYGeometryLayer->setVisible(y);
  d->rotateZGeometryLayer->setVisible(z);
}

//##################################################################################################
void GizmoLayer::setEnableTranslation(bool x, bool y, bool z)
{
  d->translateXGeometryLayer->setVisible(x);
  d->translateYGeometryLayer->setVisible(y);
  d->translateZGeometryLayer->setVisible(z);
}

//##################################################################################################
void GizmoLayer::setScale(const glm::vec3& scale)
{
  if(glm::distance2(d->scale, scale) > 0.0001f)
  {
    d->scale = scale;
    d->updateRotationGeometry = true;
    d->updateTranslationGeometry = true;
    update();
  }
}

//##################################################################################################
void GizmoLayer::setRingHeight(float ringHeight)
{
  d->ringHeight = ringHeight;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setRingRadius(float outerRadius, float innerRadius, float spikeRadius)
{
  d->outerRadius = outerRadius;
  d->innerRadius = innerRadius;
  d->spikeRadius = spikeRadius;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::render(RenderInfo& renderInfo)
{
  if(d->updateRotationGeometry)
  {
    d->updateRotationGeometry = false;
    d->generateRotationGeometry();
  }

  if(d->updateTranslationGeometry)
  {
    d->updateTranslationGeometry = false;
    d->generateTranslationGeometry();
  }

  tp_maps::Layer::render(renderInfo);
}

//##################################################################################################
bool GizmoLayer::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
  case MouseEventType::Press:
  {
    if(event.button != Button::LeftButton)
      return false;

    PickingResult* result = map()->performPicking(gizmoLayerSID(), event.pos);
    TP_CLEANUP([&]{delete result;});

    if(result)
    {
      d->previousPos = event.pos;

      if(result->layer == d->rotateXGeometryLayer)
      {
        d->activeModification = Modify_lt::RotateX;
        return true;
      }

      if(result->layer == d->rotateYGeometryLayer)
      {
        d->activeModification = Modify_lt::RotateY;
        return true;
      }

      if(result->layer == d->rotateZGeometryLayer)
      {
        d->activeModification = Modify_lt::RotateZ;
        return true;
      }

      if(result->layer == d->translateXGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslateX;
        return true;
      }

      if(result->layer == d->translateYGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslateY;
        return true;
      }

      if(result->layer == d->translateZGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslateZ;
        return true;
      }
    }
    break;
  }

  case MouseEventType::Release:
  {
    if(d->activeModification != Modify_lt::None)
    {
      d->activeModification = Modify_lt::None;
      return true;
    }
    break;
  }

  case MouseEventType::Move:
  {
    if(d->activeModification == Modify_lt::None)
      break;

    glm::ivec2 delta = event.pos - d->previousPos;

    auto mat = modelMatrix();

    auto rotate = [&](const glm::vec3& axis)
    {
      float angle = float((std::abs(delta.y)>std::abs(delta.x))?delta.y:delta.x) / 3.0f;
      mat = glm::rotate(mat, glm::radians(angle), axis);
      d->previousPos = event.pos;
    };

    auto translate = [&](const glm::vec3& axis)
    {
      auto m = map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();
      auto im = glm::inverse(m);

      //Calculate the best plane to drag the gizmo on
      tp_math_utils::Plane plane;
      {
        glm::vec4 sa = im * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec4 sb = im * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        glm::vec3 s = glm::normalize((glm::vec3(sa)/sa.w) - (glm::vec3(sb)/sb.w));

        glm::vec3 up1{axis.z, axis.x, axis.y};
        glm::vec3 up2{axis.y, axis.z, axis.x};
        plane = tp_math_utils::Plane({0.0f, 0.0f, 0.0f}, (std::fabs(glm::dot(up1, s))<std::fabs(glm::dot(up2, s)))?up1:up2);
      }

      // Screen point of the origin of the gizmo
      glm::vec2 screenPoint;
      map()->project({0.0f, 0.0f, 0.0f}, screenPoint, m);

      // Calculate the amount of translation required
      glm::vec3 a;
      glm::vec3 b;
      map()->unProject(screenPoint, a, plane, m);
      map()->unProject(screenPoint + glm::vec2(delta), b, plane, m);
      glm::vec3 translation = (b-a) * axis;

      // Validate that the translated gizmo is still within the view frustum
      auto mb = glm::translate(m, translation);
      glm::vec4 bs = mb*glm::vec4(b, 1.0f); bs /= bs.w;
      if(std::fabs(bs.x) < 1.0f && std::fabs(bs.y) < 1.0f && bs.z>0.0f && bs.z < 1.0f)
      {
        mat = glm::translate(mat, (b-a) * axis);
        d->previousPos = event.pos;
      }
    };

    switch(d->activeModification)
    {
    case Modify_lt::RotateX: rotate({1,0,0}); break;
    case Modify_lt::RotateY: rotate({0,1,0}); break;
    case Modify_lt::RotateZ: rotate({0,0,1}); break;
    case Modify_lt::TranslateX: translate({1,0,0}); break;
    case Modify_lt::TranslateY: translate({0,1,0}); break;
    case Modify_lt::TranslateZ: translate({0,0,1}); break;
    default: break;
    }

    mat[0] = glm::vec4(glm::normalize(glm::vec3(mat[0])), 0.0f);
    mat[1] = glm::vec4(glm::normalize(glm::vec3(mat[1])), 0.0f);

    mat[2] = glm::vec4(glm::cross(glm::vec3(mat[0]), glm::vec3(mat[1])), 0.0f);
    mat[1] = glm::vec4(glm::cross(glm::vec3(mat[2]), glm::vec3(mat[0])), 0.0f);

    setModelMatrix(mat);
    changed();

    return true;
  }

  default:
    break;
  }

  return Layer::mouseEvent(event);
}

}
