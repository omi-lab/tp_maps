#include "tp_maps/controllers/GraphController.h"

#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"

#include "glm/gtx/transform.hpp"

namespace tp_maps
{
//##################################################################################################
struct GraphController::Private
{
  TP_REF_COUNT_OBJECTS("GraphController::Private");
  TP_NONCOPYABLE(Private);

  GraphController* q;

  double distanceX{10.0};
  double distanceY{1.0};

  glm::dvec3 focalPoint{0, 0.5f, 0};

  //Behaviour
  bool allowTranslation{true};
  bool allowZoom{true};

  double rotationFactor{0.2};

  glm::ivec2 previousPos{0,0};
  glm::ivec2 previousPos2{0,0};
  Button mouseInteraction{Button::NoButton};
  bool mouseMoved{false};

  //################################################################################################
  Private(GraphController* q_):
    q(q_)
  {

  }

  //################################################################################################
  void translate(double dx, double dy)
  {
    double radians = glm::radians(0.0);
    double ca = std::cos(radians);
    double sa = std::sin(radians);

    //The width and height of the map widget
    double width  = double(q->map()->width());
    double height = double(q->map()->height());

    dx = dx / width;
    dy = dy / height;

    double fh = 1.0;
    double fw = 1.0;
    if(width>height)
    {
      fw = width/height;
    }
    else
    {
      fh = height/width;
    }

    dx *= (fw*distanceX)*2.0;
    dy *= fh*distanceY*2.0;

    focalPoint.x -= dx*ca - dy*sa;
    focalPoint.y += dx*sa + dy*ca;
  }
};

//##################################################################################################
GraphController::GraphController(Map* map):
  Controller(map),
  d(new Private(this))
{

}

//##################################################################################################
glm::dvec3 GraphController::focalPoint()const
{
  return d->focalPoint;
}

//##################################################################################################
void GraphController::setFocalPoint(const glm::dvec3& focalPoint)
{
  d->focalPoint = focalPoint;
  map()->update();
}

//##################################################################################################
bool GraphController::allowTranslation()const
{
  return d->allowTranslation;
}

//##################################################################################################
void GraphController::setAllowTranslation(bool allowTranslation)
{
  d->allowTranslation = allowTranslation;
}

//##################################################################################################
bool GraphController::allowZoom()const
{
  return d->allowZoom;
}

//##################################################################################################
void GraphController::setAllowZoom(bool allowZoom)
{
  d->allowZoom = allowZoom;
}

//##################################################################################################
double GraphController::rotationFactor()const
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
  map()->update();
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
  map()->update();
}

//##################################################################################################
nlohmann::json GraphController::saveState()const
{
  nlohmann::json j;

  j["Focal point"]    = tp_math_utils::vec3ToJSON(d->focalPoint);
  j["DistanceX"]      = d->distanceX;
  j["DistanceY"]      = d->distanceY;

  return j;
}

//##################################################################################################
void GraphController::loadState(const nlohmann::json& j)
{
  d->focalPoint    = tp_math_utils::getJSONVec3   (j, "Focal point"   , d->focalPoint   );
  d->distanceX     = tp_utils::getJSONValue<double>(j, "DistanceX"     , d->distanceX    );
  d->distanceY     = tp_utils::getJSONValue<double>(j, "DistanceY"     , d->distanceY    );

  map()->update();
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
}

//##################################################################################################
void GraphController::updateMatrices()
{
  if(std::fabs(d->distanceX) < 0.000000001)
    d->distanceX = 1.0;

  if(std::fabs(d->distanceY) < 0.000000001)
    d->distanceY = 1.0;

  //The width and height of the map widget
  double width  = double(map()->width());
  double height = double(map()->height());

  double fh = 1.0f;
  double fw = 1.0f;
  if(width>height)
    fw = width/height;
  else
    fh = height/width;

  glm::dmat4 view = glm::dmat4(1.0);
  view = glm::translate(view, -d->focalPoint);

  glm::dmat4 projection = glm::ortho(float(-fw*d->distanceX), // <- Left
                                    float(fw*d->distanceX),  // <- Right
                                    float(-fh*d->distanceY), // <- Bottom
                                    float(fh*d->distanceY),  // <- Top
                                    -1.0f,            // <- Near
                                    1.0f);            // <- Far
  Controller::Matrices vp;
  vp.dp  = projection;
  vp.dv  = view;
  vp.dvp = projection * view;
  vp.p  = projection;
  vp.v  = view;
  vp.vp = projection * view;
  {
    glm::vec4 origin = glm::inverse(vp.vp) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    vp.cameraOriginNear = origin / origin.w;
  }
  {
    glm::vec4 origin = glm::inverse(vp.vp) * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    vp.cameraOriginFar = origin / origin.w;
  }
  setMatrices(defaultSID(), vp);
}

//##################################################################################################
bool GraphController::mouseEvent(const MouseEvent& event)
{
  const int mouseSensitivity=8;
  switch(event.type)
  {
  case MouseEventType::Press: //--------------------------------------------------------------------
  {
    translateInteractionStarted();

    if(d->mouseInteraction == Button::NoButton)
    {
      d->mouseInteraction = event.button;
      d->previousPos = event.pos;
      d->mouseMoved = false;
    }
    break;
  }

  case MouseEventType::Move: //---------------------------------------------------------------------
  {
    glm::ivec2 pos = event.pos;

    if(!d->mouseMoved)
    {
      int ox = abs(d->previousPos.x - pos.x);
      int oy = abs(d->previousPos.y - pos.y);
      if((ox+oy) <= mouseSensitivity)
        break;
      d->mouseMoved = true;
    }

    double dx = double(pos.x - d->previousPos.x);
    double dy = double(pos.y - d->previousPos.y);

    d->previousPos2 = d->previousPos;
    d->previousPos = pos;

    if(d->mouseInteraction == Button::RightButton)
    {
      map()->update();
    }
    else if(d->mouseInteraction == Button::LeftButton && d->allowTranslation)
    {
      translate(dx, dy, 1);
      map()->update();
    }

    break;
  }

  case MouseEventType::Release: //------------------------------------------------------------------
  {
    if(event.button == d->mouseInteraction)
    {
      d->mouseInteraction = Button::NoButton;

      if(!d->mouseMoved)
      {
        int ox = abs(d->previousPos.x - event.pos.x);
        int oy = abs(d->previousPos.y - event.pos.y);
        if((ox+oy) <= mouseSensitivity)
        {
          MouseEvent e = event;
          e.type = MouseEventType::Click;
          callMouseClickCallback(e);
        }
      }
      else if(event.button == Button::LeftButton)
      {
        translateInteractionFinished();
      }
    }
    break;
  }

  case MouseEventType::Wheel: //--------------------------------------------------------------------
  {
    if(!d->allowZoom)
      return true;

    glm::dvec3 scenePointA;
    bool moveOrigin = map()->unProject(event.pos, scenePointA, tp_math_utils::Plane());

    if(d->mouseInteraction == Button::RightButton)
    {
      if(event.delta<0)
        d->distanceY *= 1.1;
      else if(event.delta>0)
        d->distanceY *= 0.9;
      else
        return true;
    }
    else
    {
      if(event.delta<0)
        d->distanceX *= 1.1;
      else if(event.delta>0)
        d->distanceX *= 0.9;
      else
        return true;
    }

    if(moveOrigin)
    {
      updateMatrices();
      glm::dvec3 scenePointB;
      moveOrigin = map()->unProject(event.pos, scenePointB, tp_math_utils::Plane());

      if(moveOrigin)
        d->focalPoint += scenePointA - scenePointB;
    }

    map()->update();
    break;
  }

  case MouseEventType::DoubleClick: //--------------------------------------------------------------
  {
    d->mouseInteraction = Button::NoButton;
    break;
  }

  default: //---------------------------------------------------------------------------------------
  {
    d->mouseInteraction = Button::NoButton;
    break;
  }
  }

  return true;
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

}
