#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/PickingResult.h"

#include "tp_math_utils/Plane.h"

#include "glm/gtx/norm.hpp" // IWYU pragma: keep

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
  TranslationX,
  TranslationY,
  TranslationZ,
  ScaleX,
  ScaleY,
  ScaleZ
};
}

//##################################################################################################
struct GizmoLayer::Private
{
  Geometry3DLayer* rotateXGeometryLayer{nullptr};
  Geometry3DLayer* rotateYGeometryLayer{nullptr};
  Geometry3DLayer* rotateZGeometryLayer{nullptr};

  Geometry3DLayer* translationXGeometryLayer{nullptr};
  Geometry3DLayer* translationYGeometryLayer{nullptr};
  Geometry3DLayer* translationZGeometryLayer{nullptr};

  Geometry3DLayer* scaleXGeometryLayer{nullptr};
  Geometry3DLayer* scaleYGeometryLayer{nullptr};
  Geometry3DLayer* scaleZGeometryLayer{nullptr};

  RotationRingStyle rotationRingStyle{RotationRingStyle::Compass};

  glm::vec3 rotationColorX{1,0,0};
  glm::vec3 rotationColorY{0,1,0};
  glm::vec3 rotationColorZ{0,0,1};

  glm::vec3 translationColorX{1,0,0};
  glm::vec3 translationColorY{0,1,0};
  glm::vec3 translationColorZ{0,0,1};

  glm::vec3 scaleColorX{1,0,0};
  glm::vec3 scaleColorY{0,1,0};
  glm::vec3 scaleColorZ{0,0,1};

  Modify_lt activeModification{Modify_lt::None};
  glm::ivec2 previousPos;
  glm::vec3 intersectionPoint;
  bool useIntersection{false};
  glm::mat4 originalModelMatrix;
  glm::mat4 originalModelToWorldMatrix;

  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 coreSize{1.0f, 1.0f, 1.0f};
  float ringHeight{0.01f};
  float outerRadius{1.00f};
  float innerRadius{0.95f};
  float spikeRadius{0.90f};
  float arrowInnerRadius{0.90f};
  float arrowOuterRadius{1.05f};


  bool updateRotationGeometry{true};
  bool updateTranslationGeometry{true};
  bool updateScaleGeometry{true};

  //################################################################################################
  void generateRotationGeometry()
  {
    auto makeCircle = [&](std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const glm::vec3& color)
    {
      auto& circle = geometry.emplace_back();

      circle.material.albedo = color;

      circle.triangleFan   = GL_TRIANGLE_FAN;
      circle.triangleStrip = GL_TRIANGLE_STRIP;
      circle.triangles     = GL_TRIANGLES;

      switch(rotationRingStyle)
      {
      case RotationRingStyle::Compass: //-----------------------------------------------------------
      {
        circle.indexes.resize(4);
        auto& top = circle.indexes.at(0);
        auto& btm = circle.indexes.at(1);
        auto& out = circle.indexes.at(2);
        auto& mid = circle.indexes.at(3);

        top.type = circle.triangleStrip;
        btm.type = circle.triangleStrip;
        out.type = circle.triangleStrip;
        mid.type = circle.triangleStrip;

        for(size_t a=0; a<=360; a+=6)
        {
          float x = std::sin(glm::radians(float(a)));
          float y = std::cos(glm::radians(float(a)));

          glm::vec2 v{x, y};

          tp_math_utils::Vertex3D vert;

          vert.vert = transform(glm::vec3(v*outerRadius, ringHeight));
          circle.verts.push_back(vert);
          vert.vert = transform(glm::vec3(v*outerRadius, -ringHeight));
          circle.verts.push_back(vert);

          auto iRad = (a%20)?innerRadius:spikeRadius;

          vert.vert = transform(glm::vec3(v*iRad, ringHeight));
          circle.verts.push_back(vert);
          vert.vert = transform(glm::vec3(v*iRad, -ringHeight));
          circle.verts.push_back(vert);

          int i = int(circle.verts.size());

          top.indexes.push_back(i-4);
          top.indexes.push_back(i-2);

          btm.indexes.push_back(i-1);
          btm.indexes.push_back(i-3);

          out.indexes.push_back(i-3);
          out.indexes.push_back(i-4);

          mid.indexes.push_back(i-2);
          mid.indexes.push_back(i-1);
        }
        break;
      }

      case RotationRingStyle::ArrowsCW:   //--------------------------------------------------------
      [[fallthrough]];
      case RotationRingStyle::ArrowsCCW: //---------------------------------------------------------
      {
        size_t stemStart=0;
        size_t arrowStart=60;
        size_t arrowEnd=80;

        float midRadius = (outerRadius + innerRadius) / 2.0f;

        //Used to hold spare triangles.
        circle.indexes.emplace_back().type = circle.triangles;

        tp_math_utils::Indexes3D* top{nullptr};
        tp_math_utils::Indexes3D* btm{nullptr};
        tp_math_utils::Indexes3D* out{nullptr};
        tp_math_utils::Indexes3D* mid{nullptr};

        for(size_t a=0; a<=360;)
        {
          //4 arrows
          size_t aa = a%90;

          if(aa == stemStart)
          {
            circle.indexes.resize(circle.indexes.size()+4);
            top = &circle.indexes.at(circle.indexes.size()-4);
            btm = &circle.indexes.at(circle.indexes.size()-3);
            out = &circle.indexes.at(circle.indexes.size()-2);
            mid = &circle.indexes.at(circle.indexes.size()-1);

            top->type = circle.triangleStrip;
            btm->type = circle.triangleStrip;
            out->type = circle.triangleStrip;
            mid->type = circle.triangleStrip;
          }

          float x = std::sin(glm::radians(float(a)));
          float y = std::cos(glm::radians(float(a)));

          glm::vec2 v{x, y};

          tp_math_utils::Vertex3D vert;

          if(aa == arrowEnd)
          {
            vert.vert = transform(glm::vec3(v*midRadius, ringHeight));
            circle.verts.push_back(vert);
            vert.vert = transform(glm::vec3(v*midRadius, -ringHeight));
            circle.verts.push_back(vert);

            {
              int i = int(circle.verts.size());

              top->indexes.push_back(i-2);

              btm->indexes.push_back(i-1);

              out->indexes.push_back(i-1);
              out->indexes.push_back(i-2);

              mid->indexes.push_back(i-2);
              mid->indexes.push_back(i-1);

              circle.indexes.front().indexes.push_back(i-2);
              circle.indexes.front().indexes.push_back(i-8);
              circle.indexes.front().indexes.push_back(i-4);

              circle.indexes.front().indexes.push_back(i-2);
              circle.indexes.front().indexes.push_back(i-6);
              circle.indexes.front().indexes.push_back(i-10);

              circle.indexes.front().indexes.push_back(i-1);
              circle.indexes.front().indexes.push_back(i-3);
              circle.indexes.front().indexes.push_back(i-7);

              circle.indexes.front().indexes.push_back(i-1);
              circle.indexes.front().indexes.push_back(i-9);
              circle.indexes.front().indexes.push_back(i-5);

            }
          }
          else
          {
            vert.vert = transform(glm::vec3(v*outerRadius, ringHeight));
            circle.verts.push_back(vert);
            vert.vert = transform(glm::vec3(v*outerRadius, -ringHeight));
            circle.verts.push_back(vert);

            vert.vert = transform(glm::vec3(v*innerRadius, ringHeight));
            circle.verts.push_back(vert);
            vert.vert = transform(glm::vec3(v*innerRadius, -ringHeight));
            circle.verts.push_back(vert);

            {
              int i = int(circle.verts.size());

              if(aa == stemStart)
              {
                out->indexes.push_back(i-1);
                out->indexes.push_back(i-2);
              }

              top->indexes.push_back(i-4);
              top->indexes.push_back(i-2);

              btm->indexes.push_back(i-1);
              btm->indexes.push_back(i-3);

              out->indexes.push_back(i-3);
              out->indexes.push_back(i-4);

              mid->indexes.push_back(i-2);
              mid->indexes.push_back(i-1);
            }

            if(aa==arrowStart)
            {
              vert.vert = transform(glm::vec3(v*arrowOuterRadius, ringHeight));
              circle.verts.push_back(vert);
              vert.vert = transform(glm::vec3(v*arrowOuterRadius, -ringHeight));
              circle.verts.push_back(vert);

              vert.vert = transform(glm::vec3(v*arrowInnerRadius, ringHeight));
              circle.verts.push_back(vert);
              vert.vert = transform(glm::vec3(v*arrowInnerRadius, -ringHeight));
              circle.verts.push_back(vert);

              int i = int(circle.verts.size());

              out->indexes.push_back(i-3);
              out->indexes.push_back(i-4);

              mid->indexes.push_back(i-2);
              mid->indexes.push_back(i-1);
            }
          }

          if(aa==arrowStart)
            a = (a-aa) + arrowEnd;

          else if(aa==arrowEnd)
            a = (a-aa) + 90;

          else
            a+=5;
        }

        if(rotationRingStyle == RotationRingStyle::ArrowsCCW)
        {
          glm::mat4 m{1.0f};
          m = glm::rotate(m, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
          circle.transform(m);
        }
        break;
      }
      }

      circle.calculateFaceNormals();
    };

    std::vector<tp_math_utils::Geometry3D> rotateXGeometry;
    std::vector<tp_math_utils::Geometry3D> rotateYGeometry;
    std::vector<tp_math_utils::Geometry3D> rotateZGeometry;

    makeCircle(rotateXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, rotationColorX);
    makeCircle(rotateYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, rotationColorY);
    makeCircle(rotateZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, rotationColorZ);

    rotateXGeometryLayer->setGeometry(rotateXGeometry);
    rotateYGeometryLayer->setGeometry(rotateYGeometry);
    rotateZGeometryLayer->setGeometry(rotateZGeometry);
  }

  //################################################################################################
  void makeArrow(std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, float stemLength, const glm::vec3& color)
  {
    auto& arrow = geometry.emplace_back();

    arrow.material.albedo = color;
    arrow.material.roughness = 0.0f;

    arrow.triangleFan   = GL_TRIANGLE_FAN;
    arrow.triangleStrip = GL_TRIANGLE_STRIP;
    arrow.triangles     = GL_TRIANGLES;

    float stemRadius = 0.05f;
    float coneRadius = 0.1f;
    float stemStart  = 0.1f;
    float stemEnd    = stemStart + stemLength;
    float coneEnd    = stemEnd + 0.2f;

    tp_math_utils::Vertex3D vert;

    // Point
    vert.vert = transform(glm::vec3(0.0f, 0.0f, coneEnd));
    arrow.verts.push_back(vert);

    // Origin
    vert.vert = transform(glm::vec3(0.0f, 0.0f, stemStart));
    arrow.verts.push_back(vert);

    arrow.indexes.resize(1);
    auto& indexes = arrow.indexes.at(0);
    indexes.type = arrow.triangles;

    for(size_t a=0; a<=360; a+=10)
    {
      float x = std::sin(glm::radians(float(a)));
      float y = std::cos(glm::radians(float(a)));

      glm::vec2 v{x, y};

      vert.vert = transform(glm::vec3(v*stemRadius, stemStart));
      arrow.verts.push_back(vert);

      vert.vert = transform(glm::vec3(v*stemRadius, stemEnd));
      arrow.verts.push_back(vert);

      vert.vert = transform(glm::vec3(v*coneRadius, stemEnd));
      arrow.verts.push_back(vert);

      int i = int(arrow.verts.size());

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

    arrow.calculateFaceNormals();
  }

  //################################################################################################
  void generateTranslationGeometry()
  {
    std::vector<tp_math_utils::Geometry3D> translationXGeometry;
    std::vector<tp_math_utils::Geometry3D> translationYGeometry;
    std::vector<tp_math_utils::Geometry3D> translationZGeometry;

    makeArrow(translationXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, 0.7f, translationColorX);
    makeArrow(translationYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, 0.7f, translationColorY);
    makeArrow(translationZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, 0.7f, translationColorZ);

    translationXGeometryLayer->setGeometry(translationXGeometry);
    translationYGeometryLayer->setGeometry(translationYGeometry);
    translationZGeometryLayer->setGeometry(translationZGeometry);
  }

  //################################################################################################
  void generateScaleGeometry()
  {
    std::vector<tp_math_utils::Geometry3D> scaleXGeometry;
    std::vector<tp_math_utils::Geometry3D> scaleYGeometry;
    std::vector<tp_math_utils::Geometry3D> scaleZGeometry;

    makeArrow(scaleXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y) * scale + glm::vec3(coreSize.x, 0.0f, 0.0f);}, 0.3f, scaleColorX);
    makeArrow(scaleYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x) * scale + glm::vec3(0.0f, coreSize.y, 0.0f);}, 0.3f, scaleColorY);
    makeArrow(scaleZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z) * scale + glm::vec3(0.0f, 0.0f, coreSize.z);}, 0.3f, scaleColorZ);

    scaleXGeometryLayer->setGeometry(scaleXGeometry);
    scaleYGeometryLayer->setGeometry(scaleYGeometry);
    scaleZGeometryLayer->setGeometry(scaleZGeometry);
  }
};

//##################################################################################################
GizmoLayer::GizmoLayer():
  d(new Private())
{
  auto createLayer = [&](auto& l)
  {
    l = new Geometry3DLayer();
    addChildLayer(l);
    l->setShaderSelection(Geometry3DLayer::ShaderSelection::StaticLight);
  };

  createLayer(d->rotateXGeometryLayer);
  createLayer(d->rotateYGeometryLayer);
  createLayer(d->rotateZGeometryLayer);

  createLayer(d->translationXGeometryLayer);
  createLayer(d->translationYGeometryLayer);
  createLayer(d->translationZGeometryLayer);

  createLayer(d->scaleXGeometryLayer);
  createLayer(d->scaleYGeometryLayer);
  createLayer(d->scaleZGeometryLayer);
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
  d->translationXGeometryLayer->setVisible(x);
  d->translationYGeometryLayer->setVisible(y);
  d->translationZGeometryLayer->setVisible(z);
}

//##################################################################################################
void GizmoLayer::setEnableScale(bool x, bool y, bool z)
{
  d->scaleXGeometryLayer->setVisible(x);
  d->scaleYGeometryLayer->setVisible(y);
  d->scaleZGeometryLayer->setVisible(z);
}

//##################################################################################################
void GizmoLayer::setRotationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->rotationColorX = x;
  d->rotationColorY = y;
  d->rotationColorZ = z;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setTranslationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->translationColorX = x;
  d->translationColorY = y;
  d->translationColorZ = z;
  d->updateTranslationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setScaleColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->scaleColorX = x;
  d->scaleColorY = y;
  d->scaleColorZ = z;
  d->updateScaleGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setScale(const glm::vec3& scale)
{
  if(glm::distance2(d->scale, scale) > 0.0001f)
  {
    d->scale = scale;
    d->updateRotationGeometry = true;
    d->updateTranslationGeometry = true;
    d->updateScaleGeometry = true;
    update();
  }
}

//##################################################################################################
void GizmoLayer::setCoreSize(const glm::vec3& coreSize)
{
  if(glm::distance2(d->coreSize, coreSize) > 0.0001f)
  {
    d->coreSize = coreSize;
    d->updateScaleGeometry = true;
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
void GizmoLayer::setRingRadius(float outerRadius,
                               float innerRadius,
                               float spikeRadius,
                               float arrowInnerRadius,
                               float arrowOuterRadius)
{
  d->outerRadius = outerRadius;
  d->innerRadius = innerRadius;
  d->spikeRadius = spikeRadius;
  d->arrowInnerRadius = arrowInnerRadius;
  d->arrowOuterRadius = arrowOuterRadius;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setRotationRingStyle(RotationRingStyle rotationRingStyle)
{
  d->rotationRingStyle = rotationRingStyle;
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

  if(d->updateScaleGeometry)
  {
    d->updateScaleGeometry = false;
    d->generateScaleGeometry();
  }

  tp_maps::Layer::render(renderInfo);
}

//##################################################################################################
bool GizmoLayer::mouseEvent(const MouseEvent& event)
{
  auto intersectPlane = [&](const glm::vec3& axis, glm::vec3& intersectionPoint)
  {
    auto m = map()->controller()->matrix(coordinateSystem()) * d->originalModelToWorldMatrix;
    tp_math_utils::Plane plane(glm::vec3(0,0,0), axis);
    return map()->unProject(event.pos, intersectionPoint, plane, m);
  };

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
      d->originalModelMatrix = modelMatrix();
      d->originalModelToWorldMatrix = modelToWorldMatrix();

      if(result->layer == d->rotateXGeometryLayer)
      {
        d->useIntersection = intersectPlane({1,0,0}, d->intersectionPoint);
        d->activeModification = Modify_lt::RotateX;
        return true;
      }

      if(result->layer == d->rotateYGeometryLayer)
      {
        d->useIntersection = intersectPlane({0,1,0}, d->intersectionPoint);
        d->activeModification = Modify_lt::RotateY;
        return true;
      }

      if(result->layer == d->rotateZGeometryLayer)
      {
        d->useIntersection = intersectPlane({0,0,1}, d->intersectionPoint);
        d->activeModification = Modify_lt::RotateZ;
        return true;
      }

      if(result->layer == d->translationXGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslationX;
        return true;
      }

      if(result->layer == d->translationYGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslationY;
        return true;
      }

      if(result->layer == d->translationZGeometryLayer)
      {
        d->activeModification = Modify_lt::TranslationZ;
        return true;
      }

      if(result->layer == d->scaleXGeometryLayer)
      {
        d->activeModification = Modify_lt::ScaleX;
        return true;
      }

      if(result->layer == d->scaleYGeometryLayer)
      {
        d->activeModification = Modify_lt::ScaleY;
        return true;
      }

      if(result->layer == d->scaleZGeometryLayer)
      {
        d->activeModification = Modify_lt::ScaleZ;
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
      float angle=0.0f;
      if(d->useIntersection)
      {
        glm::vec3 intersectionPoint;
        if(intersectPlane(axis, intersectionPoint))
        {
          mat = d->originalModelMatrix;
          auto a = glm::normalize(d->intersectionPoint);
          auto b = glm::normalize(intersectionPoint);
          angle = glm::degrees(std::acos(glm::dot(a, b)));
          if(glm::dot(axis, glm::cross(a, b))<0.0f)
            angle = -angle;
        }
      }
      else
      {
        angle = float((std::abs(delta.y)>std::abs(delta.x))?delta.y:delta.x) / 3.0f;
      }

      mat = glm::rotate(mat, glm::radians(angle), axis);
      d->previousPos = event.pos;
    };

    auto translation = [&](const glm::vec3& axis)
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

      // Validate that the translationd gizmo is still within the view frustum
      auto mb = glm::translate(m, translation);
      glm::vec4 bs = mb*glm::vec4(b, 1.0f); bs /= bs.w;
      if(std::fabs(bs.x) < 1.0f && std::fabs(bs.y) < 1.0f && bs.z>0.0f && bs.z < 1.0f)
      {
        mat = glm::translate(mat, (b-a) * axis);
        d->previousPos = event.pos;
      }
    };

    auto scale = [&](const glm::vec3& axis)
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

      glm::vec3 c = d->coreSize * axis;
      glm::vec2 screenPoint;
      map()->project(c, screenPoint, m);

      glm::vec3 b;
      map()->unProject(screenPoint + glm::vec2(delta), b, plane, m);

      b *= axis;

      float bS = b.x + b.y + b.z;
      float cS = c.x + c.y + c.z;

      float scale = bS / cS;
      scale = std::min(std::max(scale, 0.3f), 1.1f);

      mat = glm::scale(mat, glm::vec3(scale, scale, scale));
      d->previousPos = event.pos;
    };

    switch(d->activeModification)
    {
    case Modify_lt::RotateX: rotate({1,0,0}); break;
    case Modify_lt::RotateY: rotate({0,1,0}); break;
    case Modify_lt::RotateZ: rotate({0,0,1}); break;
    case Modify_lt::TranslationX: translation({1,0,0}); break;
    case Modify_lt::TranslationY: translation({0,1,0}); break;
    case Modify_lt::TranslationZ: translation({0,0,1}); break;
    case Modify_lt::ScaleX: scale({1,0,0}); break;
    case Modify_lt::ScaleY: scale({0,1,0}); break;
    case Modify_lt::ScaleZ: scale({0,0,1}); break;
    case Modify_lt::None: break;
    }

    float lX = glm::length(glm::vec3(mat[0]));
    float lY = glm::length(glm::vec3(mat[1]));
    float lZ = glm::length(glm::vec3(mat[2]));

    mat[0] = glm::vec4(glm::normalize(glm::vec3(mat[0])), 0.0f);
    mat[1] = glm::vec4(glm::normalize(glm::vec3(mat[1])), 0.0f);

    mat[2] = glm::vec4(glm::cross(glm::vec3(mat[0]), glm::vec3(mat[1])), 0.0f);
    mat[1] = glm::vec4(glm::cross(glm::vec3(mat[2]), glm::vec3(mat[0])), 0.0f);

    mat[0] *= lX;
    mat[1] *= lY;
    mat[2] *= lZ;

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
