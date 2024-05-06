#include "tp_maps/layers/GizmoLayer.h"
#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Controller.h"
#include "tp_maps/Map.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/materials/OpenGLMaterial.h"
#include "tp_math_utils/ClosestPointsOnRays.h"
#include "tp_math_utils/Ray.h"

#include "glm/gtx/norm.hpp" // IWYU pragma: keep
#include "glm/gtx/vector_angle.hpp" // IWYU pragma: keep
#include "glm/gtx/matrix_decompose.hpp" // IWYU pragma: keep
#include "glm/gtx/component_wise.hpp" // IWYU pragma: keep

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
  RotateScreen,
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
  Q* q;

  Geometry3DLayer* rotateXGeometryLayer{nullptr};
  Geometry3DLayer* rotateYGeometryLayer{nullptr};
  Geometry3DLayer* rotateZGeometryLayer{nullptr};

  Geometry3DLayer* rotateScreenGeometryLayer{nullptr};

  Geometry3DLayer* translationXGeometryLayer{nullptr};
  Geometry3DLayer* translationYGeometryLayer{nullptr};
  Geometry3DLayer* translationZGeometryLayer{nullptr};

  Geometry3DLayer* scaleXGeometryLayer{nullptr};
  Geometry3DLayer* scaleYGeometryLayer{nullptr};
  Geometry3DLayer* scaleZGeometryLayer{nullptr};

  bool rotateXGeometryLayerVisible{true};
  bool rotateYGeometryLayerVisible{true};
  bool rotateZGeometryLayerVisible{true};

  bool rotateScreenGeometryLayerVisible{false};

  bool translationXGeometryLayerVisible{true};
  bool translationYGeometryLayerVisible{true};
  bool translationZGeometryLayerVisible{true};

  bool scaleXGeometryLayerVisible{true};
  bool scaleYGeometryLayerVisible{true};
  bool scaleZGeometryLayerVisible{true};

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

  glm::vec3 selectedColor{1,1,0};
  bool useSelectedColor{false};
  bool selectedColorSubscribed{false};

  Modify_lt activeModification{Modify_lt::None};
  glm::ivec2 previousPos;
  glm::vec3 intersectionPoint;
  bool useIntersection{false};
  glm::mat4 originalModelMatrix;
  glm::mat4 originalModelToWorldMatrix;
  glm::vec3 originalScreenRotateAxis{0.0f,0.0f, 1.0f};

  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 coreSize{1.0f, 1.0f, 1.0f};
  float ringHeight{0.01f};
  float outerRadius{1.00f};
  float innerRadius{0.95f};
  float spikeRadius{0.90f};
  float arrowInnerRadius{0.90f};
  float arrowOuterRadius{1.05f};

  GizmoScaleMode gizmoScaleMode{GizmoScaleMode::Object};
  float gizmoScale{1.0f};
  bool onlyRenderSelectedAxis{false};

  bool updateRotationGeometry{true};
  bool updateTranslationGeometry{true};
  bool updateScaleGeometry{true};

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  void setMaterial(tp_math_utils::Geometry3D& mesh, const glm::vec3& color)
  {
    auto material = new tp_math_utils::OpenGLMaterial();
    mesh.material.extendedMaterials.push_back(material);

    mesh.material.name = defaultSID();
    material->albedo = color;
    material->roughness = 0.0f;
  }

  //################################################################################################
  void generateRotationGeometry()
  {
    auto makeCircle = [&](std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const glm::vec3& color)
    {
      auto& circle = geometry.emplace_back();
      setMaterial(circle, color);

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

    std::vector<tp_math_utils::Geometry3D> rotateScreenGeometry;

    makeCircle(rotateXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, rotationColorX);
    makeCircle(rotateYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, rotationColorY);
    makeCircle(rotateZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, rotationColorZ);

    makeCircle(rotateScreenGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale*1.1f;}, rotationColorZ);

    rotateXGeometryLayer->setGeometry(rotateXGeometry);
    rotateYGeometryLayer->setGeometry(rotateYGeometry);
    rotateZGeometryLayer->setGeometry(rotateZGeometry);

    rotateScreenGeometryLayer->setGeometry(rotateScreenGeometry);
  }

  //################################################################################################
  void makeArrow(std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, float stemLength, const glm::vec3& color)
  {
    auto& arrow = geometry.emplace_back();
    setMaterial(arrow, color);

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

  //################################################################################################
  void setActiveModification(Modify_lt activeModification)
  {
    if(this->activeModification == activeModification)
      return;

    this->activeModification = activeModification;
    updateVisibility();
    updateColors();
  }

  //################################################################################################
  void updateColors()
  {
    rotateXGeometryLayer     ->setAlternativeMaterials({});
    rotateYGeometryLayer     ->setAlternativeMaterials({});
    rotateZGeometryLayer     ->setAlternativeMaterials({});

    rotateScreenGeometryLayer->setAlternativeMaterials({});

    translationXGeometryLayer->setAlternativeMaterials({});
    translationYGeometryLayer->setAlternativeMaterials({});
    translationZGeometryLayer->setAlternativeMaterials({});

    scaleXGeometryLayer      ->setAlternativeMaterials({});
    scaleYGeometryLayer      ->setAlternativeMaterials({});
    scaleZGeometryLayer      ->setAlternativeMaterials({});

    if(useSelectedColor)
    {
      auto useSelected = [&](Geometry3DLayer* layer)
      {
        layer->setAlternativeMaterials({{defaultSID(), selectedSID()}});
      };

      switch(activeModification)
      {
        case Modify_lt::None         :                                         break;

        case Modify_lt::RotateX      : useSelected(rotateXGeometryLayer     ); break;
        case Modify_lt::RotateY      : useSelected(rotateYGeometryLayer     ); break;
        case Modify_lt::RotateZ      : useSelected(rotateZGeometryLayer     ); break;

        case Modify_lt::RotateScreen : useSelected(rotateScreenGeometryLayer); break;

        case Modify_lt::TranslationX : useSelected(translationXGeometryLayer); break;
        case Modify_lt::TranslationY : useSelected(translationYGeometryLayer); break;
        case Modify_lt::TranslationZ : useSelected(translationZGeometryLayer); break;

        case Modify_lt::ScaleX       : useSelected(scaleXGeometryLayer      ); break;
        case Modify_lt::ScaleY       : useSelected(scaleYGeometryLayer      ); break;
        case Modify_lt::ScaleZ       : useSelected(scaleZGeometryLayer      ); break;
      }
    }
  }

  //################################################################################################
  void updateVisibility()
  {
    if(!onlyRenderSelectedAxis || activeModification == Modify_lt::None)
    {
      rotateXGeometryLayer     ->setVisible(rotateXGeometryLayerVisible     );
      rotateYGeometryLayer     ->setVisible(rotateYGeometryLayerVisible     );
      rotateZGeometryLayer     ->setVisible(rotateZGeometryLayerVisible     );

      rotateScreenGeometryLayer->setVisible(rotateScreenGeometryLayerVisible);

      translationXGeometryLayer->setVisible(translationXGeometryLayerVisible);
      translationYGeometryLayer->setVisible(translationYGeometryLayerVisible);
      translationZGeometryLayer->setVisible(translationZGeometryLayerVisible);

      scaleXGeometryLayer      ->setVisible(scaleXGeometryLayerVisible      );
      scaleYGeometryLayer      ->setVisible(scaleYGeometryLayerVisible      );
      scaleZGeometryLayer      ->setVisible(scaleZGeometryLayerVisible      );
    }
    else
    {
      rotateXGeometryLayer     ->setVisible(rotateXGeometryLayerVisible      && activeModification == Modify_lt::RotateX);
      rotateYGeometryLayer     ->setVisible(rotateYGeometryLayerVisible      && activeModification == Modify_lt::RotateY);
      rotateZGeometryLayer     ->setVisible(rotateZGeometryLayerVisible      && activeModification == Modify_lt::RotateZ);

      rotateScreenGeometryLayer->setVisible(rotateScreenGeometryLayerVisible && activeModification == Modify_lt::RotateScreen);

      translationXGeometryLayer->setVisible(translationXGeometryLayerVisible && activeModification == Modify_lt::TranslationX);
      translationYGeometryLayer->setVisible(translationYGeometryLayerVisible && activeModification == Modify_lt::TranslationY);
      translationZGeometryLayer->setVisible(translationZGeometryLayerVisible && activeModification == Modify_lt::TranslationZ);

      scaleXGeometryLayer      ->setVisible(scaleXGeometryLayerVisible       && activeModification == Modify_lt::ScaleX);
      scaleYGeometryLayer      ->setVisible(scaleYGeometryLayerVisible       && activeModification == Modify_lt::ScaleY);
      scaleZGeometryLayer      ->setVisible(scaleZGeometryLayerVisible       && activeModification == Modify_lt::ScaleZ);
    }

    q->update();
  }

  //################################################################################################
  glm::vec3 screenRotateAxis()
  {
    return glm::normalize(tpProj(rotateScreenGeometryLayer->modelMatrix(), {0.0f, 0.0f, 1.0f}));
  }
};

//##################################################################################################
GizmoLayer::GizmoLayer():
  d(new Private(this))
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

  createLayer(d->rotateScreenGeometryLayer);

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
bool GizmoLayer::inInteraction() const
{
  return d->activeModification != Modify_lt::None;
}

//##################################################################################################
void GizmoLayer::setEnableRotation(bool x, bool y, bool z)
{
  d->rotateXGeometryLayerVisible = x;
  d->rotateYGeometryLayerVisible = y;
  d->rotateZGeometryLayerVisible = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableRotationScreen(bool screen)
{
  d->rotateScreenGeometryLayerVisible = screen;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableTranslation(bool x, bool y, bool z)
{
  d->translationXGeometryLayerVisible = x;
  d->translationYGeometryLayerVisible = y;
  d->translationZGeometryLayerVisible = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableScale(bool x, bool y, bool z)
{
  d->scaleXGeometryLayerVisible = x;
  d->scaleYGeometryLayerVisible = y;
  d->scaleZGeometryLayerVisible = z;
  d->updateVisibility();
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
void GizmoLayer::setSelectedColor(const glm::vec3& selectedColor)
{
  d->selectedColor = selectedColor;
  d->useSelectedColor = true;

  auto updateMaterial = [&](Geometry3DLayer* layer)
  {
    if(d->selectedColorSubscribed)
      layer->geometry3DPool()->unsubscribe(selectedSID());

    layer->geometry3DPool()->subscribe(selectedSID(), [=]
    {
      std::vector<tp_math_utils::Geometry3D> geometry;
      d->setMaterial(geometry.emplace_back(), selectedColor);
      return geometry;
    }, true, true);
  };

  updateMaterial(d->rotateXGeometryLayer);
  updateMaterial(d->rotateYGeometryLayer);
  updateMaterial(d->rotateZGeometryLayer);

  updateMaterial(d->rotateScreenGeometryLayer);

  updateMaterial(d->translationXGeometryLayer);
  updateMaterial(d->translationYGeometryLayer);
  updateMaterial(d->translationZGeometryLayer);

  updateMaterial(d->scaleXGeometryLayer);
  updateMaterial(d->scaleYGeometryLayer);
  updateMaterial(d->scaleZGeometryLayer);

  d->selectedColorSubscribed = true;
  d->updateColors();
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
void GizmoLayer::setGizmoScaleMode(GizmoScaleMode gizmoScaleMode)
{
  d->gizmoScaleMode = gizmoScaleMode;
  update();
}

//##################################################################################################
void GizmoLayer::setGizmoScale(float gizmoScale)
{
  d->gizmoScale = gizmoScale;
  update();
}

//##################################################################################################
void GizmoLayer::setOnlyRenderSelectedAxis(bool onlyRenderSelectedAxis)
{
  d->onlyRenderSelectedAxis = onlyRenderSelectedAxis;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setDefaultRenderPass(const RenderPass& defaultRenderPass)
{
  d->rotateXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->rotateYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->rotateZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->rotateScreenGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->translationXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->scaleXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->scaleYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->scaleZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  Layer::setDefaultRenderPass(defaultRenderPass);
}

//##################################################################################################
glm::mat4 matrixToRotateAOntoB(const glm::vec3& a, const glm::vec3& b)
{
  auto alpha = glm::angle(a, b);

  if(std::fabs(alpha) > 1e-3f)
  {
    auto v = glm::normalize(glm::cross(a, b));
    return glm::rotate(glm::mat4(1.0f), alpha, v);
  }

  return glm::mat4(1.0f);
}

//##################################################################################################
void GizmoLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass == RenderPass::PreRender)
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

    {
      tp_maps::Matrices matrices = map()->controller()->matrices(coordinateSystem());
      glm::mat4 modelToWorld = modelToWorldMatrix();

      float s{1.0f};
      switch(d->gizmoScaleMode)
      {
        case GizmoScaleMode::World:

        break;

        case GizmoScaleMode::Object:
        {
          s *= d->gizmoScale;
          break;
        }

        case GizmoScaleMode::Screen:

        break;

        case GizmoScaleMode::ScreenPX:
        {
          auto m = matrices.vp * modelToWorld;

          glm::vec3 screenPointO;
          map()->project({0.0f, 0.0f, 0.0f}, screenPointO, m);

          glm::vec3 delta{0.707f, 0.707f, 0.0f};
          delta = delta*d->gizmoScale;

          glm::vec3 screenPointB = screenPointO + delta;

          glm::vec3 vec = map()->unProject(screenPointB, m);

          s = glm::length(vec);

          break;
        }
      }

      glm::mat4 mScale = glm::scale(glm::mat4{1.0f}, glm::vec3{s,s,s});

      if(d->rotateScreenGeometryLayer->visible())
      {
        glm::vec3 axis{0,0,1};
        glm::vec3 forward;
        {
          glm::mat4 vpInv = glm::inverse(matrices.vp*modelToWorld);
          glm::vec3 a = tpProj(vpInv, {0.0f, 0.0f, 0.0f});
          glm::vec3 b = tpProj(vpInv, {0.0f, 0.0f, 1.0f});
          forward = glm::normalize(b-a);
        }

        glm::mat4 mRot = matrixToRotateAOntoB(axis, forward);

        d->rotateScreenGeometryLayer->setModelMatrix(mScale * mRot);
      }

      {
        d->rotateXGeometryLayer->setModelMatrix(mScale);
        d->rotateYGeometryLayer->setModelMatrix(mScale);
        d->rotateZGeometryLayer->setModelMatrix(mScale);

        d->translationXGeometryLayer->setModelMatrix(mScale);
        d->translationYGeometryLayer->setModelMatrix(mScale);
        d->translationZGeometryLayer->setModelMatrix(mScale);

        d->scaleXGeometryLayer->setModelMatrix(mScale);
        d->scaleYGeometryLayer->setModelMatrix(mScale);
        d->scaleZGeometryLayer->setModelMatrix(mScale);
      }
    }
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
          d->setActiveModification(Modify_lt::RotateX);
          return true;
        }

        if(result->layer == d->rotateYGeometryLayer)
        {
          d->useIntersection = intersectPlane({0,1,0}, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateY);
          return true;
        }

        if(result->layer == d->rotateZGeometryLayer)
        {
          d->useIntersection = intersectPlane({0,0,1}, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateZ);
          return true;
        }

        if(result->layer == d->rotateScreenGeometryLayer)
        {
          d->originalScreenRotateAxis = d->screenRotateAxis();
          d->useIntersection = intersectPlane(d->originalScreenRotateAxis, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateScreen);
          return true;
        }

        if(result->layer == d->translationXGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationX);
          return true;
        }

        if(result->layer == d->translationYGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationY);
          return true;
        }

        if(result->layer == d->translationZGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationZ);
          return true;
        }

        if(result->layer == d->scaleXGeometryLayer)
        {
          d->setActiveModification(Modify_lt::ScaleX);
          return true;
        }

        if(result->layer == d->scaleYGeometryLayer)
        {
          d->setActiveModification(Modify_lt::ScaleY);
          return true;
        }

        if(result->layer == d->scaleZGeometryLayer)
        {
          d->setActiveModification(Modify_lt::ScaleZ);
          return true;
        }
      }
      break;
    }

    case MouseEventType::Release:
    {
      if(d->activeModification != Modify_lt::None)
      {
        d->setActiveModification(Modify_lt::None);
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

        tp_math_utils::DRay rayAxis({0.0, 0.0, 0.0}, {axis});

        auto calculateClosestPointOnAxis = [&](const glm::ivec2& pos)
        {
          glm::vec3 a{float(pos.x), float(pos.y), 0.0f};
          glm::vec3 b{float(pos.x), float(pos.y), 1.0f};

          a = map()->unProject(a, m);
          b = map()->unProject(b, m);

          glm::dvec3 P1;
          glm::dvec3 P2;

          tp_math_utils::closestPointsOnRays(rayAxis, tp_math_utils::DRay({a}, {b}), P1, P2);

          return glm::vec3(P1);
        };

       glm::vec3 pointOld = calculateClosestPointOnAxis(d->previousPos);
       glm::vec3 pointNew = calculateClosestPointOnAxis(event.pos);

       glm::vec3 translation = pointNew - pointOld;

       mat = d->originalModelMatrix;
       mat = glm::translate(mat, translation);
      };

      auto scale = [&](const glm::vec3& axis)
      {
        auto m = map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();

        tp_math_utils::DRay rayAxis({0.0, 0.0, 0.0}, {axis});

        auto calculateClosestPointOnAxis = [&](const glm::ivec2& pos)
        {
          glm::vec3 a{float(pos.x), float(pos.y), 0.0f};
          glm::vec3 b{float(pos.x), float(pos.y), 1.0f};

          a = map()->unProject(a, m);
          b = map()->unProject(b, m);

          glm::dvec3 P1;
          glm::dvec3 P2;

          tp_math_utils::closestPointsOnRays(rayAxis, tp_math_utils::DRay({a}, {b}), P1, P2);

          return glm::vec3(P1);
        };

       glm::vec3 pointOld = calculateClosestPointOnAxis(d->previousPos);
       glm::vec3 pointNew = calculateClosestPointOnAxis(event.pos);

       float scale = glm::compAdd(pointNew) / glm::compAdd(pointOld);

       float maxScale = 6.0f;

       scale = std::clamp(scale, 1.0f/maxScale, maxScale);

       mat = d->originalModelMatrix;
       mat = glm::scale(mat, {scale, scale, scale});
      };

      switch(d->activeModification)
      {
        case Modify_lt::RotateX: rotate({1,0,0}); break;
        case Modify_lt::RotateY: rotate({0,1,0}); break;
        case Modify_lt::RotateZ: rotate({0,0,1}); break;
        case Modify_lt::RotateScreen: rotate(d->originalScreenRotateAxis); break;
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
