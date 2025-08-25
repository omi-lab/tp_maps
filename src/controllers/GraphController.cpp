#include "tp_maps/controllers/GraphController.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"

#include "glm/gtx/transform.hpp" // IWYU pragma: keep

namespace tp_maps
{
//##################################################################################################
struct GraphController::Private
{
  TP_REF_COUNT_OBJECTS("GraphController::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  double distanceX{10.0};
  double distanceY{1.0};

  double limitDistanceMinX{0.0000001};
  double limitDistanceMaxX{1000000.0};

  double limitDistanceMinY{0.0000001};
  double limitDistanceMaxY{1000000.0};

  glm::dvec3 focalPoint{0, 0.5f, 0};

  double limitFocalPointMinX{-1000000.0};
  double limitFocalPointMaxX{ 1000000.0};

  double limitFocalPointMinY{-1000000.0};
  double limitFocalPointMaxY{ 1000000.0};

  //Behaviour
  bool allowTranslation{true};
  bool allowZoom{true};
  bool ignoreAspectRatio{false};

  double rotationFactor{0.2};

  glm::ivec2 previousPos{0,0};
  glm::ivec2 previousPos2{0,0};
  Button mouseInteraction{Button::NoButton};

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  struct S
  {
    double width{0.0};
    double height{0.0};
    double fw{1.0};
    double fh{1.0};
  };

  //################################################################################################
  S screenShape()
  {
    S s;

    s.width  = double(q->map()->width());
    s.height = double(q->map()->height());

    s.fh = 1.0;
    s.fw = 1.0;

    if(!ignoreAspectRatio)
    {
      if(s.width>s.height)
        s.fw = s.width/s.height;
      else
        s.fh = s.height/s.width;
    }

    return s;
  }

  //################################################################################################
  void translate(double dx, double dy)
  {
    double radians = glm::radians(0.0);
    double ca = std::cos(radians);
    double sa = std::sin(radians);

    S s = screenShape();

    dx = dx / s.width;
    dy = dy / s.height;

    dx *= s.fw*distanceX*2.0;
    dy *= s.fh*distanceY*2.0;

    focalPoint.x -= dx*ca - dy*sa;
    focalPoint.y += dx*sa + dy*ca;

    clampFocalPoint();
  }

  //################################################################################################
  void clampFocalPoint()
  {
    focalPoint.x = std::clamp(focalPoint.x, limitFocalPointMinX, limitFocalPointMaxX);
    focalPoint.y = std::clamp(focalPoint.y, limitFocalPointMinY, limitFocalPointMaxY);
    q->focalPointParametersChanged();
  }
};

//##################################################################################################
GraphController::GraphController(Map* map):
  Controller(map),
  d(new Private(this))
{

}

//##################################################################################################
glm::dvec3 GraphController::focalPoint() const
{
  return d->focalPoint;
}

//##################################################################################################
void GraphController::setFocalPoint(const glm::dvec3& focalPoint)
{
  d->focalPoint = focalPoint;
  update();
}

//##################################################################################################
double GraphController::limitFocalPointMinX() const
{
  return d->limitFocalPointMinX;
}

//##################################################################################################
double GraphController::limitFocalPointMaxX() const
{
  return d->limitFocalPointMaxX;
}

//##################################################################################################
void GraphController::setLimitFocalPointX(double minX, double maxX) const
{
  d->limitFocalPointMinX = minX;
  d->limitFocalPointMaxX = maxX;
  d->clampFocalPoint();
}

//##################################################################################################
double GraphController::limitFocalPointMinY() const
{
  return d->limitFocalPointMinY;
}

//##################################################################################################
double GraphController::limitFocalPointMaxY() const
{
  return d->limitFocalPointMaxY;
}

//##################################################################################################
void GraphController::setLimitFocalPointY(double minY, double maxY) const
{
  d->limitFocalPointMinY = minY;
  d->limitFocalPointMaxY = maxY;
  d->clampFocalPoint();
}

//##################################################################################################
bool GraphController::allowTranslation() const
{
  return d->allowTranslation;
}

//##################################################################################################
void GraphController::setAllowTranslation(bool allowTranslation)
{
  d->allowTranslation = allowTranslation;
}

//##################################################################################################
bool GraphController::allowZoom() const
{
  return d->allowZoom;
}

//##################################################################################################
void GraphController::setAllowZoom(bool allowZoom)
{
  d->allowZoom = allowZoom;
}

//##################################################################################################
double GraphController::rotationFactor() const
{
  return d->rotationFactor;
}

//##################################################################################################
void GraphController::setRotationFactor(double rotationFactor)
{
  d->rotationFactor = rotationFactor;
}

//##################################################################################################
double GraphController::distanceX() const
{
  return d->distanceX;
}

//##################################################################################################
void GraphController::setDistanceX(double distanceX)
{
  d->distanceX = distanceX;
  update();
}

//##################################################################################################
double GraphController::distanceY() const
{
  return d->distanceY;
}

//##################################################################################################
void GraphController::setDistanceY(double distanceY)
{
  d->distanceY = distanceY;
  update();
}

//##################################################################################################
void GraphController::setLimitDistanceX(double limitDistanceMinX, double limitDistanceMaxX) const
{
  d->limitDistanceMinX = limitDistanceMinX;
  d->limitDistanceMaxX = limitDistanceMaxX;
}

//##################################################################################################
double GraphController::limitDistanceMinX() const
{
  return d->limitDistanceMinX;
}

//##################################################################################################
double GraphController::limitDistanceMaxX() const
{
  return d->limitDistanceMaxX;
}

//##################################################################################################
void GraphController::setLimitDistanceY(double limitDistanceMinY, double limitDistanceMaxY) const
{
  d->limitDistanceMinY = limitDistanceMinY;
  d->limitDistanceMaxY = limitDistanceMaxY;
}

//##################################################################################################
double GraphController::limitDistanceMinY() const
{
  return d->limitDistanceMinY;
}

//##################################################################################################
double GraphController::limitDistanceMaxY() const
{
  return d->limitDistanceMinY;
}

//##################################################################################################
bool GraphController::ignoreAspectRatio() const
{
  return d->ignoreAspectRatio;
}

//##################################################################################################
void GraphController::setIgnoreAspectRatio(bool ignoreAspectRatio)
{
  d->ignoreAspectRatio = ignoreAspectRatio;
  update();
}

//##################################################################################################
void GraphController::saveState(nlohmann::json& j) const
{
  j["Focal point"]    = tp_math_utils::vec3ToJSON(d->focalPoint);
  j["DistanceX"]      = d->distanceX;
  j["DistanceY"]      = d->distanceY;
}

//##################################################################################################
void GraphController::loadState(const nlohmann::json& j)
{
  d->focalPoint    = tp_math_utils::getJSONVec3(j, "Focal point"   , d->focalPoint);
  d->distanceX     = TPJSONDouble              (j, "DistanceX"     , d->distanceX );
  d->distanceY     = TPJSONDouble              (j, "DistanceY"     , d->distanceY );

  update();
}

//##################################################################################################
GraphController::~GraphController()
{
  delete d;
}

//##################################################################################################
void GraphController::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
  d->clampFocalPoint();
}

//##################################################################################################
void GraphController::updateMatrices()
{
  if(std::fabs(d->distanceX) < 0.000000001)
    d->distanceX = 1.0;

  if(std::fabs(d->distanceY) < 0.000000001)
    d->distanceY = 1.0;

  auto s = d->screenShape();

  glm::dmat4 view = glm::dmat4(1.0);
  view = glm::translate(view, -d->focalPoint);

  glm::dmat4 projection = glm::ortho(float(-s.fw*d->distanceX), // <- Left
                                     float(s.fw*d->distanceX) , // <- Right
                                     float(-s.fh*d->distanceY), // <- Bottom
                                     float(s.fh*d->distanceY) , // <- Top
                                     -1.0f                    , // <- Near
                                     1.0f);                     // <- Far
  Matrices vp;
  vp.dp  = projection;
  vp.dv  = view;
  vp.dvp = projection * view;
  vp.p  = projection;
  vp.v  = view;
  vp.vp = projection * view;
  setMatrices(defaultSID(), vp);
}

//##################################################################################################
bool GraphController::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
    case MouseEventType::DragStart: //--------------------------------------------------------------
    {
      translateInteractionStarted();

      if(d->mouseInteraction == Button::NoButton)
      {
        d->mouseInteraction = event.button;
        d->previousPos = event.pos;
      }
      return true;
    }

    case MouseEventType::Move: //-------------------------------------------------------------------
    {
      glm::ivec2 pos = event.pos;

      double dx = double(pos.x - d->previousPos.x);
      double dy = double(pos.y - d->previousPos.y);

      d->previousPos2 = d->previousPos;
      d->previousPos = pos;

      if(d->mouseInteraction == Button::RightButton)
      {
        update();
        return true;
      }
      else if(d->mouseInteraction == Button::LeftButton && d->allowTranslation)
      {
        translate(dx, dy, 1);
        update();
        return true;
      }

      break;
    }

    case MouseEventType::Release: //----------------------------------------------------------------
    {
      if(event.button == d->mouseInteraction)
      {
        d->mouseInteraction = Button::NoButton;
        if(event.button == Button::LeftButton)
          translateInteractionFinished();

        return true;
      }
      break;
    }

    case MouseEventType::Wheel: //------------------------------------------------------------------
    {
      if(!d->allowZoom)
        return true;


      glm::dvec3 scenePointA;
      bool moveOrigin = map()->unProject(event.pos, scenePointA, tp_math_utils::Plane());

      if(d->mouseInteraction == Button::RightButton || tp_maps::keyboardModifierAnySet(event.modifiers, tp_maps::KeyboardModifier::Control | tp_maps::KeyboardModifier::Shift))
      {
        if(event.delta<0)
          d->distanceY *= 1.1;
        else if(event.delta>0)
          d->distanceY *= 0.9;
        else
          return true;

        d->distanceY = std::clamp(d->distanceY, d->limitDistanceMinY, d->limitDistanceMaxY);
      }
      else
      {
        if(event.delta<0)
          d->distanceX *= 1.1;
        else if(event.delta>0)
          d->distanceX *= 0.9;
        else
          return true;

        d->distanceX = std::clamp(d->distanceX, d->limitDistanceMinX, d->limitDistanceMaxX);
      }

      zoomPerformed();

      if(moveOrigin)
      {
        updateMatrices();
        glm::dvec3 scenePointB;
        moveOrigin = map()->unProject(event.pos, scenePointB, tp_math_utils::Plane());

        if(moveOrigin)
          d->focalPoint += scenePointA - scenePointB;
      }


      d->clampFocalPoint();
      update();
      return true;
    }

    case MouseEventType::DoubleClick: //------------------------------------------------------------
    {
      d->mouseInteraction = Button::NoButton;
      break;
    }

    default: //-------------------------------------------------------------------------------------
    {
      d->mouseInteraction = Button::NoButton;
      break;
    }
  }

  return false;
}

//##################################################################################################
void GraphController::translate(double dx, double dy, double msSincePrevious)
{
  TP_UNUSED(msSincePrevious);
  d->translate(dx, dy);
}

//##################################################################################################
void GraphController::translateInteractionStarted()
{
}

//##################################################################################################
void GraphController::translateInteractionFinished()
{
}

//##################################################################################################
void GraphController::zoomPerformed()
{
}

}
