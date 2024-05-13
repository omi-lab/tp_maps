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
#include "tp_math_utils/Intersection.h"
#include "tp_math_utils/JSONUtils.h"

#include "tp_utils/JSONUtils.h"

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
  PlaneTranslationX,
  PlaneTranslationY,
  PlaneTranslationZ,
  PlaneTranslationScreen,
  ScaleX,
  ScaleY,
  ScaleZ
};

}

//##################################################################################################
std::vector<std::string> gizmoRingStyles()
{
  return {"Compass", "ArrowsCW", "ArrowsCCW"};
}

//##################################################################################################
std::string gizmoRingStyleToString(GizmoRingStyle style)
{
  switch(style)
  {
    case GizmoRingStyle::Compass  : return "Compass"  ;
    case GizmoRingStyle::ArrowsCW : return "ArrowsCW" ;
    case GizmoRingStyle::ArrowsCCW: return "ArrowsCCW";
  }

  return "Compass";
}

//##################################################################################################
GizmoRingStyle gizmoRingStyleFromString(const std::string& style)
{
  if(style == "Compass"  ) return GizmoRingStyle::Compass  ;
  if(style == "ArrowsCW" ) return GizmoRingStyle::ArrowsCW ;
  if(style == "ArrowsCCW") return GizmoRingStyle::ArrowsCCW;

  return GizmoRingStyle::Compass;
}

//##################################################################################################
void GizmoRingParameters::saveState(nlohmann::json& j) const
{
  j["color"] = tp_math_utils::vec3ToJSON(color);
  j["selectedColor"] = tp_math_utils::vec3ToJSON(selectedColor);
  j["enable"] = enable;
  j["useSelectedColor"] = useSelectedColor;
  j["style"] = gizmoRingStyleToString(style);
}

//##################################################################################################
void GizmoRingParameters::loadState(const nlohmann::json& j)
{
  color = tp_math_utils::getJSONVec3(j, "color", {1.0f, 0.0f, 0.0f});
  selectedColor = tp_math_utils::getJSONVec3(j, "selectedColor", {1.0f, 1.0f, 0.0f});
  enable = TPJSONBool(j, "enable", true);
  useSelectedColor = TPJSONBool(j, "useSelectedColor", false);
  style = gizmoRingStyleFromString(TPJSONString(j, "style", "Compass"));
}

//##################################################################################################
std::vector<std::string> gizmoArrowStyles()
{
  return {"None", "Stem", "Stemless"};
}

//##################################################################################################
std::string gizmoArrowStyleToString(GizmoArrowStyle style)
{
  switch(style)
  {
    case GizmoArrowStyle::None    : return "None"    ;
    case GizmoArrowStyle::Stem    : return "Stem"    ;
    case GizmoArrowStyle::Stemless: return "Stemless";
  }
  return "None";
}

//##################################################################################################
GizmoArrowStyle gizmoArrowStyleFromString(const std::string& style)
{
  if(style == "None"    ) return GizmoArrowStyle::None    ;
  if(style == "Stem"    ) return GizmoArrowStyle::Stem    ;
  if(style == "Stemless") return GizmoArrowStyle::Stemless;

  return GizmoArrowStyle::None;
}

//##################################################################################################
void GizmoArrowParameters::saveState(nlohmann::json& j) const
{
  j["color"] = tp_math_utils::vec3ToJSON(color);
  j["selectedColor"] = tp_math_utils::vec3ToJSON(selectedColor);
  j["enable"] = enable;
  j["useSelectedColor"] = useSelectedColor;
  j["stemStart"] = stemStart;
  j["stemLength"] = stemLength;
  j["stemRadius"] = stemRadius;
  j["coneRadius"] = coneRadius;
  j["coneLength"] = coneLength;
  j["positiveArrowStyle"] = gizmoArrowStyleToString(positiveArrowStyle);
  j["negativeArrowStyle"] = gizmoArrowStyleToString(negativeArrowStyle);
}

//##################################################################################################
void GizmoArrowParameters::loadState(const nlohmann::json& j)
{
  color = tp_math_utils::getJSONVec3(j, "color", {1.0f, 0.0f, 0.0f});
  selectedColor = tp_math_utils::getJSONVec3(j, "selectedColor", {1.0f, 1.0f, 0.0f});
  enable = TPJSONBool(j, "enable", true);
  useSelectedColor = TPJSONBool(j, "useSelectedColor", false);

  stemStart  = TPJSONFloat(j, "stemStart" , 0.1f );
  stemLength = TPJSONFloat(j, "stemLength", 0.7f );
  stemRadius = TPJSONFloat(j, "stemRadius", 0.05f);
  coneRadius = TPJSONFloat(j, "coneRadius", 0.1f );
  coneLength = TPJSONFloat(j, "coneLength", 0.2f );

  positiveArrowStyle = gizmoArrowStyleFromString(TPJSONString(j, "positiveArrowStyle", "Stem"));
  negativeArrowStyle = gizmoArrowStyleFromString(TPJSONString(j, "negativeArrowStyle", "None"));
}

//##################################################################################################
void GizmoPlaneParameters::saveState(nlohmann::json& j) const
{
  j["color"] = tp_math_utils::vec3ToJSON(color);
  j["selectedColor"] = tp_math_utils::vec3ToJSON(selectedColor);
  j["enable"] = enable;
  j["useSelectedColor"] = useSelectedColor;
  j["size"] = size;
  j["radius"] = radius;
  j["padding"] = padding;
  j["center"] = center;
}

//##################################################################################################
void GizmoPlaneParameters::loadState(const nlohmann::json& j)
{
  color = tp_math_utils::getJSONVec3(j, "color", {1.0f, 0.0f, 0.0f});
  selectedColor = tp_math_utils::getJSONVec3(j, "selectedColor", {1.0f, 1.0f, 0.0f});
  enable = TPJSONBool(j, "enable", true);
  useSelectedColor = TPJSONBool(j, "useSelectedColor", false);

  size    = TPJSONFloat(j, "size"   , 0.5f );
  radius  = TPJSONFloat(j, "radius" , 0.3f );
  padding = TPJSONFloat(j, "padding", 0.40f);

  center = TPJSONBool(j, "center", false);
}

//##################################################################################################
std::vector<std::string> gizmoScaleModes()
{
  return {"World", "Object", "Screen", "ScreenPX"};
}

//##################################################################################################
std::string gizmoScaleModeToString(GizmoScaleMode mode)
{
  switch(mode)
  {
    case GizmoScaleMode::World   : return "World"   ;
    case GizmoScaleMode::Object  : return "Object"  ;
    case GizmoScaleMode::Screen  : return "Screen"  ;
    case GizmoScaleMode::ScreenPX: return "ScreenPX";
  }

  return "Object";
}

//##################################################################################################
GizmoScaleMode gizmoScaleModeFromString(const std::string& mode)
{
  if(mode == "World"   ) return GizmoScaleMode::World   ;
  if(mode == "Object"  ) return GizmoScaleMode::Object  ;
  if(mode == "Screen"  ) return GizmoScaleMode::Screen  ;
  if(mode == "ScreenPX") return GizmoScaleMode::ScreenPX;

  return GizmoScaleMode::Object;
}

//##################################################################################################
std::vector<std::string> gizmoRenderPasses()
{
  return {"Normal", "GUI3D"};
}

//##################################################################################################
std::string gizmoRenderPassToString(GizmoRenderPass renderPass)
{
  switch(renderPass)
  {
    case GizmoRenderPass::Normal: return "Normal";
    case GizmoRenderPass::GUI3D : return "GUI3D" ;
  }

  return "Normal";
}

//##################################################################################################
GizmoRenderPass gizmoRenderPassFromString(const std::string& renderPass)
{
  if(renderPass == "Normal") return GizmoRenderPass::Normal;
  if(renderPass == "GUI3D" ) return GizmoRenderPass::GUI3D ;
  return GizmoRenderPass::Normal;
}

//##################################################################################################
void GizmoParameters::saveState(nlohmann::json& j) const
{
  j["gizmoRenderPass"] = gizmoRenderPassToString(gizmoRenderPass);
  j["referenceLinesRenderPass"] = gizmoRenderPassToString(referenceLinesRenderPass);

  j["gizmoScaleMode"] = gizmoScaleModeToString(gizmoScaleMode);
  j["gizmoScale"] = gizmoScale;
  j["onlyRenderSelectedAxis"] = onlyRenderSelectedAxis;

  rotationX.saveState(j["rotationX"]);
  rotationY.saveState(j["rotationY"]);
  rotationZ.saveState(j["rotationZ"]);

  rotationScreen.saveState(j["rotationScreen"]);

  translationArrowX.saveState(j["translationArrowX"]);
  translationArrowY.saveState(j["translationArrowY"]);
  translationArrowZ.saveState(j["translationArrowZ"]);

  translationPlaneX.saveState(j["translationPlaneX"]);
  translationPlaneY.saveState(j["translationPlaneY"]);
  translationPlaneZ.saveState(j["translationPlaneZ"]);

  translationPlaneScreen.saveState(j["translationPlaneScreen"]);

  scaleArrowX.saveState(j["scaleArrowX"]);
  scaleArrowY.saveState(j["scaleArrowY"]);
  scaleArrowZ.saveState(j["scaleArrowZ"]);
}

//##################################################################################################
void GizmoParameters::loadState(const nlohmann::json& j)
{
  gizmoRenderPass = gizmoRenderPassFromString(TPJSONString(j, "gizmoRenderPass", "GUI3D"));
  referenceLinesRenderPass = gizmoRenderPassFromString(TPJSONString(j, "referenceLinesRenderPass", "Normal"));

  gizmoScaleMode = gizmoScaleModeFromString(TPJSONString(j, "gizmoScaleMode", "Object"));
  gizmoScale = TPJSONFloat(j, "gizmoScale", 1.0f);
  onlyRenderSelectedAxis = TPJSONBool(j, "onlyRenderSelectedAxis", false);

  tp_utils::loadObjectFromJSON(j, "rotationX", rotationX);
  tp_utils::loadObjectFromJSON(j, "rotationY", rotationY);
  tp_utils::loadObjectFromJSON(j, "rotationZ", rotationZ);

  tp_utils::loadObjectFromJSON(j, "rotationScreen", rotationScreen);

  tp_utils::loadObjectFromJSON(j, "translationArrowX", translationArrowX);
  tp_utils::loadObjectFromJSON(j, "translationArrowY", translationArrowY);
  tp_utils::loadObjectFromJSON(j, "translationArrowZ", translationArrowZ);

  tp_utils::loadObjectFromJSON(j, "translationPlaneX", translationPlaneX);
  tp_utils::loadObjectFromJSON(j, "translationPlaneY", translationPlaneY);
  tp_utils::loadObjectFromJSON(j, "translationPlaneZ", translationPlaneZ);

  tp_utils::loadObjectFromJSON(j, "translationPlaneScreen", translationPlaneScreen);

  tp_utils::loadObjectFromJSON(j, "scaleArrowX", scaleArrowX);
  tp_utils::loadObjectFromJSON(j, "scaleArrowY", scaleArrowY);
  tp_utils::loadObjectFromJSON(j, "scaleArrowZ", scaleArrowZ);
}

//##################################################################################################
struct GizmoLayer::Private
{
  Q* q;

  Geometry3DLayer* rotationXGeometryLayer{nullptr};
  Geometry3DLayer* rotationYGeometryLayer{nullptr};
  Geometry3DLayer* rotationZGeometryLayer{nullptr};

  Geometry3DLayer* rotationScreenGeometryLayer{nullptr};

  Geometry3DLayer* translationArrowXGeometryLayer{nullptr};
  Geometry3DLayer* translationArrowYGeometryLayer{nullptr};
  Geometry3DLayer* translationArrowZGeometryLayer{nullptr};

  Geometry3DLayer* translationPlaneXGeometryLayer{nullptr};
  Geometry3DLayer* translationPlaneYGeometryLayer{nullptr};
  Geometry3DLayer* translationPlaneZGeometryLayer{nullptr};

  Geometry3DLayer* translationPlaneScreenGeometryLayer{nullptr};

  Geometry3DLayer* scaleArrowXGeometryLayer{nullptr};
  Geometry3DLayer* scaleArrowYGeometryLayer{nullptr};
  Geometry3DLayer* scaleArrowZGeometryLayer{nullptr};

  GizmoParameters params;

  bool selectedColorSubscribed{false};

  Modify_lt activeModification{Modify_lt::None};
  glm::ivec2 previousPos;
  glm::vec3 intersectionPoint;
  bool useIntersection{false};
  glm::mat4 originalModelMatrix;
  glm::mat4 originalModelToWorldMatrix;
  glm::mat4 screenRelativeMatrix;
  glm::vec3 originalScreenRotateAxis{0.0f,0.0f, 1.0f};

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
  bool updatePlaneTranslationGeometry{true};
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
    auto makeCircle = [&](std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const GizmoRingParameters& params)
    {
      auto& circle = geometry.emplace_back();
      setMaterial(circle, params.color);

      circle.triangleFan   = GL_TRIANGLE_FAN;
      circle.triangleStrip = GL_TRIANGLE_STRIP;
      circle.triangles     = GL_TRIANGLES;

      switch(params.style)
      {
        case GizmoRingStyle::Compass: //---------------------------------------------------------
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

        case GizmoRingStyle::ArrowsCW:   //------------------------------------------------------
        [[fallthrough]];
        case GizmoRingStyle::ArrowsCCW: //-------------------------------------------------------
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

          if(params.style == GizmoRingStyle::ArrowsCCW)
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

    std::vector<tp_math_utils::Geometry3D> rotationXGeometry;
    std::vector<tp_math_utils::Geometry3D> rotationYGeometry;
    std::vector<tp_math_utils::Geometry3D> rotationZGeometry;

    std::vector<tp_math_utils::Geometry3D> rotationScreenGeometry;

    makeCircle(rotationXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, params.rotationX);
    makeCircle(rotationYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, params.rotationY);
    makeCircle(rotationZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, params.rotationZ);

    makeCircle(rotationScreenGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale*1.1f;}, params.rotationScreen);

    rotationXGeometryLayer->setGeometry(rotationXGeometry);
    rotationYGeometryLayer->setGeometry(rotationYGeometry);
    rotationZGeometryLayer->setGeometry(rotationZGeometry);

    rotationScreenGeometryLayer->setGeometry(rotationScreenGeometry);
  }



  //################################################################################################
  void makeArrow(std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const GizmoArrowParameters& params)
  {
    auto& arrow = geometry.emplace_back();
    setMaterial(arrow, params.color);

    arrow.triangleFan   = GL_TRIANGLE_FAN;
    arrow.triangleStrip = GL_TRIANGLE_STRIP;
    arrow.triangles     = GL_TRIANGLES;

    float stemRadius = params.stemRadius;
    float coneRadius = params.coneRadius;
    float stemStart  = params.stemStart;
    float stemEnd    = stemStart + params.stemLength;
    float coneEnd    = stemEnd + params.coneLength;

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

    if(params.positiveArrowStyle == GizmoArrowStyle::Stem)
    {
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
    }

    // Add the stemless arrow
    if(params.negativeArrowStyle == GizmoArrowStyle::Stemless)
    {
      // Point
      int iP = int(arrow.verts.size());
      vert.vert = transform(glm::vec3(0.0f, 0.0f, -coneEnd));
      arrow.verts.push_back(vert);

      // Middle
      int iM = int(arrow.verts.size());
      vert.vert = transform(glm::vec3(0.0f, 0.0f, -stemEnd));
      arrow.verts.push_back(vert);


      for(size_t a=0; a<=360; a+=10)
      {
        float x = std::sin(glm::radians(float(a)));
        float y = std::cos(glm::radians(float(a)));

        glm::vec2 v{x, y};

        vert.vert = transform(glm::vec3(v*coneRadius, -stemEnd));
        arrow.verts.push_back(vert);

        int i = int(arrow.verts.size());

        if(a==0)
          continue;

        indexes.indexes.push_back(iP);
        indexes.indexes.push_back(i-2);
        indexes.indexes.push_back(i-1);

        indexes.indexes.push_back(iM);
        indexes.indexes.push_back(i-1);
        indexes.indexes.push_back(i-2);
      }
    }

    arrow.calculateFaceNormals();
  }

  //################################################################################################
  void makePlane(std::vector<tp_math_utils::Geometry3D>& geometry, const std::function<glm::vec3(const glm::vec3&)>& transform, const GizmoPlaneParameters& params)
  {
    float min = 0.0f;
    float max = 1.0f;

    if(params.center)
    {
      min = -params.radius;
      max = +params.radius;
    }
    else
    {
      min = params.padding;
    }


    auto& plane = geometry.emplace_back();
    setMaterial(plane, params.color);

    plane.triangleFan   = GL_TRIANGLE_FAN;
    plane.triangleStrip = GL_TRIANGLE_STRIP;
    plane.triangles     = GL_TRIANGLES;

    tp_math_utils::Vertex3D vert;

    vert.vert = transform(glm::vec3(min, min, 0.0f) * params.size); plane.verts.push_back(vert);
    vert.vert = transform(glm::vec3(min, max, 0.0f) * params.size); plane.verts.push_back(vert);
    vert.vert = transform(glm::vec3(max, max, 0.0f) * params.size); plane.verts.push_back(vert);
    vert.vert = transform(glm::vec3(max, min, 0.0f) * params.size); plane.verts.push_back(vert);

    plane.indexes.resize(1);
    auto& indexes = plane.indexes.at(0);
    indexes.type = plane.triangles;
    indexes.indexes.reserve(12);

    indexes.indexes.push_back(0);
    indexes.indexes.push_back(1);
    indexes.indexes.push_back(2);

    indexes.indexes.push_back(2);
    indexes.indexes.push_back(3);
    indexes.indexes.push_back(0);

    indexes.indexes.push_back(0);
    indexes.indexes.push_back(2);
    indexes.indexes.push_back(1);

    indexes.indexes.push_back(2);
    indexes.indexes.push_back(0);
    indexes.indexes.push_back(3);

    plane.calculateFaceNormals();
  }

  //################################################################################################
  void generateTranslationGeometry()
  {
    std::vector<tp_math_utils::Geometry3D> translationArrowXGeometry;
    std::vector<tp_math_utils::Geometry3D> translationArrowYGeometry;
    std::vector<tp_math_utils::Geometry3D> translationArrowZGeometry;

    makeArrow(translationArrowXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, params.translationArrowX);
    makeArrow(translationArrowYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, params.translationArrowY);
    makeArrow(translationArrowZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, params.translationArrowZ);

    translationArrowXGeometryLayer->setGeometry(translationArrowXGeometry);
    translationArrowYGeometryLayer->setGeometry(translationArrowYGeometry);
    translationArrowZGeometryLayer->setGeometry(translationArrowZGeometry);
  }

  //################################################################################################
  void generatePlaneTranslationGeometry()
  {
    std::vector<tp_math_utils::Geometry3D> translationPlaneXGeometry;
    std::vector<tp_math_utils::Geometry3D> translationPlaneYGeometry;
    std::vector<tp_math_utils::Geometry3D> translationPlaneZGeometry;

    std::vector<tp_math_utils::Geometry3D> translationPlaneScreenGeometry;

    makePlane(translationPlaneXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y)*scale;}, params.translationPlaneX);
    makePlane(translationPlaneYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x)*scale;}, params.translationPlaneY);
    makePlane(translationPlaneZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, params.translationPlaneZ);

    makePlane(translationPlaneScreenGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z)*scale;}, params.translationPlaneScreen);

    translationPlaneXGeometryLayer->setGeometry(translationPlaneXGeometry);
    translationPlaneYGeometryLayer->setGeometry(translationPlaneYGeometry);
    translationPlaneZGeometryLayer->setGeometry(translationPlaneZGeometry);

    translationPlaneScreenGeometryLayer->setGeometry(translationPlaneScreenGeometry);
  }

  //################################################################################################
  void generateScaleGeometry()
  {
    std::vector<tp_math_utils::Geometry3D> scaleArrowXGeometry;
    std::vector<tp_math_utils::Geometry3D> scaleArrowYGeometry;
    std::vector<tp_math_utils::Geometry3D> scaleArrowZGeometry;

    makeArrow(scaleArrowXGeometry, [&](const auto& c){return glm::vec3(c.z, c.x, c.y) * scale + glm::vec3(coreSize.x, 0.0f, 0.0f);}, params.scaleArrowX);
    makeArrow(scaleArrowYGeometry, [&](const auto& c){return glm::vec3(c.y, c.z, c.x) * scale + glm::vec3(0.0f, coreSize.y, 0.0f);}, params.scaleArrowY);
    makeArrow(scaleArrowZGeometry, [&](const auto& c){return glm::vec3(c.x, c.y, c.z) * scale + glm::vec3(0.0f, 0.0f, coreSize.z);}, params.scaleArrowZ);

    scaleArrowXGeometryLayer->setGeometry(scaleArrowXGeometry);
    scaleArrowYGeometryLayer->setGeometry(scaleArrowYGeometry);
    scaleArrowZGeometryLayer->setGeometry(scaleArrowZGeometry);
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
    rotationXGeometryLayer     ->setAlternativeMaterials({});
    rotationYGeometryLayer     ->setAlternativeMaterials({});
    rotationZGeometryLayer     ->setAlternativeMaterials({});

    rotationScreenGeometryLayer->setAlternativeMaterials({});

    translationArrowXGeometryLayer->setAlternativeMaterials({});
    translationArrowYGeometryLayer->setAlternativeMaterials({});
    translationArrowZGeometryLayer->setAlternativeMaterials({});

    translationPlaneXGeometryLayer->setAlternativeMaterials({});
    translationPlaneYGeometryLayer->setAlternativeMaterials({});
    translationPlaneZGeometryLayer->setAlternativeMaterials({});

    translationPlaneScreenGeometryLayer->setAlternativeMaterials({});

    scaleArrowXGeometryLayer      ->setAlternativeMaterials({});
    scaleArrowYGeometryLayer      ->setAlternativeMaterials({});
    scaleArrowZGeometryLayer      ->setAlternativeMaterials({});

    auto useSelected = [&](const auto& params, Geometry3DLayer* layer)
    {
      if(params.useSelectedColor)
        layer->setAlternativeMaterials({{defaultSID(), selectedSID()}});
    };

    switch(activeModification)
    {
      case Modify_lt::None                   :                                                   break;

      case Modify_lt::RotateX                : useSelected(params.rotationX             , rotationXGeometryLayer);              break;
      case Modify_lt::RotateY                : useSelected(params.rotationY             , rotationYGeometryLayer);              break;
      case Modify_lt::RotateZ                : useSelected(params.rotationZ             , rotationZGeometryLayer);              break;

      case Modify_lt::RotateScreen           : useSelected(params.rotationScreen        , rotationScreenGeometryLayer);         break;

      case Modify_lt::TranslationX           : useSelected(params.translationArrowX     , translationArrowXGeometryLayer);      break;
      case Modify_lt::TranslationY           : useSelected(params.translationArrowY     , translationArrowYGeometryLayer);      break;
      case Modify_lt::TranslationZ           : useSelected(params.translationArrowZ     , translationArrowZGeometryLayer);      break;

      case Modify_lt::PlaneTranslationX      : useSelected(params.translationPlaneX     , translationPlaneXGeometryLayer);      break;
      case Modify_lt::PlaneTranslationY      : useSelected(params.translationPlaneY     , translationPlaneYGeometryLayer);      break;
      case Modify_lt::PlaneTranslationZ      : useSelected(params.translationPlaneZ     , translationPlaneZGeometryLayer);      break;

      case Modify_lt::PlaneTranslationScreen : useSelected(params.translationPlaneScreen, translationPlaneScreenGeometryLayer); break;

      case Modify_lt::ScaleX                 : useSelected(params.scaleArrowX           , scaleArrowXGeometryLayer);            break;
      case Modify_lt::ScaleY                 : useSelected(params.scaleArrowY           , scaleArrowYGeometryLayer);            break;
      case Modify_lt::ScaleZ                 : useSelected(params.scaleArrowZ           , scaleArrowZGeometryLayer);            break;
    }
  }

  //################################################################################################
  void updateVisibility()
  {
    if(!params.onlyRenderSelectedAxis || activeModification == Modify_lt::None)
    {
      rotationXGeometryLayer->setVisible(params.rotationX.enable);
      rotationYGeometryLayer->setVisible(params.rotationY.enable);
      rotationZGeometryLayer->setVisible(params.rotationZ.enable);

      rotationScreenGeometryLayer->setVisible(params.rotationScreen.enable);

      translationArrowXGeometryLayer->setVisible(params.translationArrowX.enable);
      translationArrowYGeometryLayer->setVisible(params.translationArrowY.enable);
      translationArrowZGeometryLayer->setVisible(params.translationArrowZ.enable);

      translationPlaneXGeometryLayer->setVisible(params.translationPlaneX.enable);
      translationPlaneYGeometryLayer->setVisible(params.translationPlaneY.enable);
      translationPlaneZGeometryLayer->setVisible(params.translationPlaneZ.enable);

      translationPlaneScreenGeometryLayer->setVisible(params.translationPlaneScreen.enable);

      scaleArrowXGeometryLayer->setVisible(params.scaleArrowX.enable);
      scaleArrowYGeometryLayer->setVisible(params.scaleArrowY.enable);
      scaleArrowZGeometryLayer->setVisible(params.scaleArrowZ.enable);
    }
    else
    {
      rotationXGeometryLayer->setVisible(params.rotationX.enable && activeModification == Modify_lt::RotateX);
      rotationYGeometryLayer->setVisible(params.rotationY.enable && activeModification == Modify_lt::RotateY);
      rotationZGeometryLayer->setVisible(params.rotationZ.enable && activeModification == Modify_lt::RotateZ);

      rotationScreenGeometryLayer->setVisible(params.rotationScreen.enable && activeModification == Modify_lt::RotateScreen);

      translationArrowXGeometryLayer->setVisible(params.translationArrowX.enable && activeModification == Modify_lt::TranslationX);
      translationArrowYGeometryLayer->setVisible(params.translationArrowY.enable && activeModification == Modify_lt::TranslationY);
      translationArrowZGeometryLayer->setVisible(params.translationArrowZ.enable && activeModification == Modify_lt::TranslationZ);

      translationPlaneXGeometryLayer->setVisible(params.translationPlaneX.enable && activeModification == Modify_lt::PlaneTranslationX);
      translationPlaneYGeometryLayer->setVisible(params.translationPlaneY.enable && activeModification == Modify_lt::PlaneTranslationY);
      translationPlaneZGeometryLayer->setVisible(params.translationPlaneZ.enable && activeModification == Modify_lt::PlaneTranslationZ);

      translationPlaneScreenGeometryLayer->setVisible(params.translationPlaneScreen.enable && activeModification == Modify_lt::PlaneTranslationScreen);

      scaleArrowXGeometryLayer->setVisible(params.scaleArrowX.enable && activeModification == Modify_lt::ScaleX);
      scaleArrowYGeometryLayer->setVisible(params.scaleArrowY.enable && activeModification == Modify_lt::ScaleY);
      scaleArrowZGeometryLayer->setVisible(params.scaleArrowZ.enable && activeModification == Modify_lt::ScaleZ);
    }

    q->update();
  }

  //################################################################################################
  glm::vec3 screenRotateAxis()
  {
    return glm::normalize(tpProj(screenRelativeMatrix, {0.0f, 0.0f, 1.0f}));
  }


  //################################################################################################
  void updateSelectedColors()
  {
    auto updateMaterial = [&](Geometry3DLayer* layer, const auto& params)
    {
      if(selectedColorSubscribed)
        layer->geometry3DPool()->unsubscribe(selectedSID());

      layer->geometry3DPool()->subscribe(selectedSID(), [=]
      {
        std::vector<tp_math_utils::Geometry3D> geometry;
        setMaterial(geometry.emplace_back(), params.selectedColor);
        return geometry;
      }, true, true);
    };

    updateMaterial(rotationXGeometryLayer, params.rotationX);
    updateMaterial(rotationYGeometryLayer, params.rotationY);
    updateMaterial(rotationZGeometryLayer, params.rotationZ);

    updateMaterial(rotationScreenGeometryLayer, params.rotationScreen);

    updateMaterial(translationArrowXGeometryLayer, params.translationArrowX);
    updateMaterial(translationArrowYGeometryLayer, params.translationArrowY);
    updateMaterial(translationArrowZGeometryLayer, params.translationArrowZ);

    updateMaterial(translationPlaneXGeometryLayer, params.translationPlaneX);
    updateMaterial(translationPlaneYGeometryLayer, params.translationPlaneY);
    updateMaterial(translationPlaneZGeometryLayer, params.translationPlaneZ);

    updateMaterial(translationPlaneScreenGeometryLayer, params.translationPlaneScreen);

    updateMaterial(scaleArrowXGeometryLayer, params.scaleArrowX);
    updateMaterial(scaleArrowYGeometryLayer, params.scaleArrowY);
    updateMaterial(scaleArrowZGeometryLayer, params.scaleArrowZ);

    selectedColorSubscribed = true;
    updateColors();
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

  createLayer(d->rotationXGeometryLayer);
  createLayer(d->rotationYGeometryLayer);
  createLayer(d->rotationZGeometryLayer);

  createLayer(d->rotationScreenGeometryLayer);

  createLayer(d->translationArrowXGeometryLayer);
  createLayer(d->translationArrowYGeometryLayer);
  createLayer(d->translationArrowZGeometryLayer);

  createLayer(d->translationPlaneXGeometryLayer);
  createLayer(d->translationPlaneYGeometryLayer);
  createLayer(d->translationPlaneZGeometryLayer);

  createLayer(d->translationPlaneScreenGeometryLayer);

  createLayer(d->scaleArrowXGeometryLayer);
  createLayer(d->scaleArrowYGeometryLayer);
  createLayer(d->scaleArrowZGeometryLayer);
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
void GizmoLayer::setParameters(const GizmoParameters& params)
{
  d->params = params;

  d->updateRotationGeometry = true;
  d->updateTranslationGeometry = true;
  d->updatePlaneTranslationGeometry = true;
  d->updateScaleGeometry = true;

  switch(d->params.gizmoRenderPass)
  {
    case GizmoRenderPass::Normal: setDefaultRenderPass(RenderPass::Normal); break;
    case GizmoRenderPass::GUI3D : setDefaultRenderPass(RenderPass::GUI3D ); break;
  }

  d->updateSelectedColors();
  d->updateVisibility();
}

//##################################################################################################
const GizmoParameters& GizmoLayer::parameters() const
{
  return d->params;
}

//##################################################################################################
void GizmoLayer::setEnableRotation(bool x, bool y, bool z)
{
  d->params.rotationX.enable = x;
  d->params.rotationY.enable = y;
  d->params.rotationZ.enable = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableRotationScreen(bool screen)
{
  d->params.rotationScreen.enable = screen;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableTranslationArrows(bool x, bool y, bool z)
{
  d->params.translationArrowX.enable = x;
  d->params.translationArrowY.enable = y;
  d->params.translationArrowZ.enable = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableTranslationPlanes(bool x, bool y, bool z)
{
  d->params.translationPlaneX.enable = x;
  d->params.translationPlaneY.enable = y;
  d->params.translationPlaneZ.enable = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableTranslationPlanesScreenScreen(bool screen)
{
  d->params.translationPlaneScreen.enable = screen;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setEnableScale(bool x, bool y, bool z)
{
  d->params.scaleArrowX.enable = x;
  d->params.scaleArrowY.enable = y;
  d->params.scaleArrowZ.enable = z;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setRotationColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->params.rotationX.color = x;
  d->params.rotationY.color = y;
  d->params.rotationZ.color = z;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setRotationScreenColor(const glm::vec3& color)
{
  d->params.rotationScreen.color = color;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setTranslationArrowColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->params.translationArrowX.color = x;
  d->params.translationArrowY.color = y;
  d->params.translationArrowZ.color = z;
  d->updateTranslationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setTranslationPlaneColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->params.translationPlaneX.color = x;
  d->params.translationPlaneY.color = y;
  d->params.translationPlaneZ.color = z;
  d->updatePlaneTranslationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setPlaneTranslationScreenColor(const glm::vec3& color)
{
  d->params.translationPlaneScreen.color = color;
  d->updatePlaneTranslationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setScaleColors(const glm::vec3& x, const glm::vec3& y, const glm::vec3& z)
{
  d->params.scaleArrowX.color = x;
  d->params.scaleArrowY.color = y;
  d->params.scaleArrowZ.color = z;
  d->updateScaleGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setSelectedColor(const glm::vec3& selectedColor)
{
  auto updateMaterial = [&](auto& params)
  {
    params.selectedColor = selectedColor;
    params.useSelectedColor = true;
  };

  updateMaterial(d->params.rotationX);
  updateMaterial(d->params.rotationY);
  updateMaterial(d->params.rotationZ);

  updateMaterial(d->params.rotationScreen);

  updateMaterial(d->params.translationArrowX);
  updateMaterial(d->params.translationArrowY);
  updateMaterial(d->params.translationArrowZ);

  updateMaterial(d->params.translationPlaneX);
  updateMaterial(d->params.translationPlaneY);
  updateMaterial(d->params.translationPlaneZ);

  updateMaterial(d->params.translationPlaneScreen);

  updateMaterial(d->params.scaleArrowX);
  updateMaterial(d->params.scaleArrowY);
  updateMaterial(d->params.scaleArrowZ);

  d->updateSelectedColors();
}

//##################################################################################################
void GizmoLayer::setScale(const glm::vec3& scale)
{
  if(glm::distance2(d->scale, scale) > 0.0001f)
  {
    d->scale = scale;
    d->updateRotationGeometry = true;
    d->updateTranslationGeometry = true;
    d->updatePlaneTranslationGeometry = true;
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
void GizmoLayer::setGizmoRingStyle(GizmoRingStyle gizmoRingStyle)
{
  d->params.rotationX.style = gizmoRingStyle;
  d->params.rotationY.style = gizmoRingStyle;
  d->params.rotationZ.style = gizmoRingStyle;
  d->params.rotationScreen.style = gizmoRingStyle;
  d->updateRotationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setGizmoScaleMode(GizmoScaleMode gizmoScaleMode)
{
  d->params.gizmoScaleMode = gizmoScaleMode;
  update();
}

//##################################################################################################
void GizmoLayer::setGizmoScale(float gizmoScale)
{
  d->params.gizmoScale = gizmoScale;
  update();
}

//##################################################################################################
void GizmoLayer::setTranslationArrowParameters(const GizmoArrowParameters& x,
                                               const GizmoArrowParameters& y,
                                               const GizmoArrowParameters& z)
{
  d->params.translationArrowX = z;
  d->params.translationArrowY = y;
  d->params.translationArrowZ = x;

  d->updateTranslationGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setScaleArrowParameters(const GizmoArrowParameters& x,
                                         const GizmoArrowParameters& y,
                                         const GizmoArrowParameters& z)
{
  d->params.scaleArrowX = z;
  d->params.scaleArrowY = y;
  d->params.scaleArrowZ = x;

  d->updateScaleGeometry = true;
  update();
}

//##################################################################################################
void GizmoLayer::setOnlyRenderSelectedAxis(bool onlyRenderSelectedAxis)
{
  d->params.onlyRenderSelectedAxis = onlyRenderSelectedAxis;
  d->updateVisibility();
}

//##################################################################################################
void GizmoLayer::setDefaultRenderPass(const RenderPass& defaultRenderPass)
{
  if(defaultRenderPass.type == RenderPass::GUI3D)
    d->params.gizmoRenderPass = GizmoRenderPass::GUI3D;
  else
    d->params.gizmoRenderPass = GizmoRenderPass::Normal;

  d->rotationXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->rotationYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->rotationZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->rotationScreenGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->translationArrowXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationArrowYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationArrowZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->translationPlaneXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationPlaneYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->translationPlaneZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->translationPlaneScreenGeometryLayer->setDefaultRenderPass(defaultRenderPass);

  d->scaleArrowXGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->scaleArrowYGeometryLayer->setDefaultRenderPass(defaultRenderPass);
  d->scaleArrowZGeometryLayer->setDefaultRenderPass(defaultRenderPass);

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

    if(d->updatePlaneTranslationGeometry)
    {
      d->updatePlaneTranslationGeometry = false;
      d->generatePlaneTranslationGeometry();
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
      switch(d->params.gizmoScaleMode)
      {
        case GizmoScaleMode::World:

        break;

        case GizmoScaleMode::Object:
        {
          s *= d->params.gizmoScale;
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
          delta = delta*d->params.gizmoScale;

          glm::vec3 screenPointB = screenPointO + delta;

          glm::vec3 vec = map()->unProject(screenPointB, m);

          s = glm::length(vec);

          break;
        }
      }

      glm::mat4 mScale = glm::scale(glm::mat4{1.0f}, glm::vec3{s,s,s});

      if(d->rotationScreenGeometryLayer->visible() ||
         d->translationPlaneScreenGeometryLayer->visible())
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

        d->screenRelativeMatrix = mScale * mRot;

        if(d->rotationScreenGeometryLayer->visible())
        {
          d->rotationScreenGeometryLayer->setModelMatrix(d->screenRelativeMatrix);
        }

        if(d->translationPlaneScreenGeometryLayer->visible())
        {
          auto m = matrices.vp * modelToWorld * d->screenRelativeMatrix;
          auto mInv = glm::inverse(m);

          glm::vec3 a = tpProj(mInv, {0.0f, 0.0f, 0.0f});
          glm::vec3 b = tpProj(mInv, {0.0f, 1.0f, 0.0f});
          glm::vec3 vUp = glm::normalize(b-a);
          glm::vec3 vModel = {0.707f, 0.707f, 0.0f};

          glm::mat4 mAlignUp = matrixToRotateAOntoB(vModel, vUp);

          d->translationPlaneScreenGeometryLayer->setModelMatrix(d->screenRelativeMatrix * mAlignUp);
        }
      }

      {
        d->rotationXGeometryLayer->setModelMatrix(mScale);
        d->rotationYGeometryLayer->setModelMatrix(mScale);
        d->rotationZGeometryLayer->setModelMatrix(mScale);

        d->translationArrowXGeometryLayer->setModelMatrix(mScale);
        d->translationArrowYGeometryLayer->setModelMatrix(mScale);
        d->translationArrowZGeometryLayer->setModelMatrix(mScale);

        d->translationPlaneXGeometryLayer->setModelMatrix(mScale);
        d->translationPlaneYGeometryLayer->setModelMatrix(mScale);
        d->translationPlaneZGeometryLayer->setModelMatrix(mScale);

        d->scaleArrowXGeometryLayer->setModelMatrix(mScale);
        d->scaleArrowYGeometryLayer->setModelMatrix(mScale);
        d->scaleArrowZGeometryLayer->setModelMatrix(mScale);
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

        if(result->layer == d->rotationXGeometryLayer)
        {
          d->useIntersection = intersectPlane({1,0,0}, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateX);
          return true;
        }

        if(result->layer == d->rotationYGeometryLayer)
        {
          d->useIntersection = intersectPlane({0,1,0}, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateY);
          return true;
        }

        if(result->layer == d->rotationZGeometryLayer)
        {
          d->useIntersection = intersectPlane({0,0,1}, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateZ);
          return true;
        }

        if(result->layer == d->rotationScreenGeometryLayer)
        {
          d->originalScreenRotateAxis = d->screenRotateAxis();
          d->useIntersection = intersectPlane(d->originalScreenRotateAxis, d->intersectionPoint);
          d->setActiveModification(Modify_lt::RotateScreen);
          return true;
        }

        if(result->layer == d->translationArrowXGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationX);
          return true;
        }

        if(result->layer == d->translationArrowYGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationY);
          return true;
        }

        if(result->layer == d->translationArrowZGeometryLayer)
        {
          d->setActiveModification(Modify_lt::TranslationZ);
          return true;
        }

        if(result->layer == d->translationPlaneXGeometryLayer)
        {
          d->setActiveModification(Modify_lt::PlaneTranslationX);
          return true;
        }

        if(result->layer == d->translationPlaneYGeometryLayer)
        {
          d->setActiveModification(Modify_lt::PlaneTranslationY);
          return true;
        }

        if(result->layer == d->translationPlaneZGeometryLayer)
        {
          d->setActiveModification(Modify_lt::PlaneTranslationZ);
          return true;
        }

        if(result->layer == d->translationPlaneScreenGeometryLayer)
        {
          d->originalScreenRotateAxis = d->screenRotateAxis();
          d->setActiveModification(Modify_lt::PlaneTranslationScreen);
          return true;
        }

        if(result->layer == d->scaleArrowXGeometryLayer)
        {
          d->setActiveModification(Modify_lt::ScaleX);
          return true;
        }

        if(result->layer == d->scaleArrowYGeometryLayer)
        {
          d->setActiveModification(Modify_lt::ScaleY);
          return true;
        }

        if(result->layer == d->scaleArrowZGeometryLayer)
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

      auto translationAlongAxis = [&](const glm::vec3& axis)
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

      auto translationOnPlane = [&](const glm::vec3& axis)
      {
        auto m = map()->controller()->matrix(coordinateSystem()) * modelToWorldMatrix();

        tp_math_utils::Plane plane({0.0f, 0.0f, 0.0f}, axis);

        auto calculateClosestPointOnPlane = [&](const glm::ivec2& pos, glm::vec3& intersection)
        {
          glm::vec3 a{float(pos.x), float(pos.y), 0.0f};
          glm::vec3 b{float(pos.x), float(pos.y), 1.0f};

          a = map()->unProject(a, m);
          b = map()->unProject(b, m);

          return tp_math_utils::rayPlaneIntersection(tp_math_utils::Ray({a}, {b}), plane, intersection);
        };

        glm::vec3 pointOld;
        glm::vec3 pointNew;

        if(!calculateClosestPointOnPlane(d->previousPos, pointOld))
          return;

        if(!calculateClosestPointOnPlane(event.pos, pointNew))
          return;

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
        case Modify_lt::TranslationX: translationAlongAxis({1,0,0}); break;
        case Modify_lt::TranslationY: translationAlongAxis({0,1,0}); break;
        case Modify_lt::TranslationZ: translationAlongAxis({0,0,1}); break;
        case Modify_lt::PlaneTranslationX: translationOnPlane({1,0,0}); break;
        case Modify_lt::PlaneTranslationY: translationOnPlane({0,1,0}); break;
        case Modify_lt::PlaneTranslationZ: translationOnPlane({0,0,1}); break;
        case Modify_lt::PlaneTranslationScreen: translationOnPlane(d->originalScreenRotateAxis); break;
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
