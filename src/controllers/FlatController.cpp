#include "tp_maps/controllers/FlatController.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"

#include "glm/gtx/transform.hpp" // IWYU pragma: keep

namespace tp_maps
{
//##################################################################################################
struct FlatController::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FlatController::Private");
  TP_NONCOPYABLE(Private);

  FlatController* q;

  float distance{10.0f};

  //The state of the camera
  float viewAngle{0.0f};
  float rotationAngle{0.0f};

  glm::vec3 focalPoint{0, 0, 0};

  //Behaviour
  bool allowRotation{true};
  bool variableViewAngle{true};
  bool allowTranslation{true};
  bool allowZoom{true};

  float rotationFactor{0.2f};

  glm::ivec2 previousPos{0,0};
  glm::ivec2 previousPos2{0,0};
  Button mouseInteraction{Button::NoButton};

  Button rotateButton{Button::RightButton};
  Button translateButton{Button::LeftButton};

  //################################################################################################
  Private(FlatController* q_):
    q(q_)
  {

  }

  //################################################################################################
  void translate(float dx, float dy)
  {
    float radians = glm::radians(rotationAngle);
    float ca = std::cos(radians);
    float sa = std::sin(radians);

    //The width and height of the map widget
    float width  = float(q->map()->width());
    float height = float(q->map()->height());

    dx = dx / width;
    dy = dy / height;

    float fh = 1.0f;
    float fw = 1.0f;
    if(width>height)
    {
      fw = width/height;
    }
    else
    {
      fh = height/width;
    }

    dx *= (fw*distance)*2.0f;
    dy *= fh*distance*2.0f;

    focalPoint.x -= dx*ca - dy*sa;
    focalPoint.y += dx*sa + dy*ca;
  }
};

//##################################################################################################
FlatController::FlatController(Map* map):
  Controller(map),
  d(new Private(this))
{

}

//##################################################################################################
glm::vec3 FlatController::focalPoint() const
{
  return d->focalPoint;
}

//##################################################################################################
void FlatController::setFocalPoint(const glm::vec3& focalPoint)
{
  d->focalPoint = focalPoint;
  update();
}

//##################################################################################################
float FlatController::distance() const
{
  return d->distance;
}

//##################################################################################################
void FlatController::setDistance(float distance)
{
  d->distance = distance;
  update();
}

//##################################################################################################
bool FlatController::allowRotation() const
{
  return d->allowRotation;
}

//##################################################################################################
void FlatController::setAllowRotation(bool allowRotation)
{
  d->allowRotation = allowRotation;
}

//##################################################################################################
bool FlatController::variableViewAngle() const
{
  return d->variableViewAngle;
}

//##################################################################################################
void FlatController::setVariableViewAngle(bool variableViewAngle)
{
  d->variableViewAngle = variableViewAngle;
}

//##################################################################################################
bool FlatController::allowTranslation() const
{
  return d->allowTranslation;
}

//##################################################################################################
void FlatController::setAllowTranslation(bool allowTranslation)
{
  d->allowTranslation = allowTranslation;
}

//##################################################################################################
bool FlatController::allowZoom() const
{
  return d->allowZoom;
}

//##################################################################################################
void FlatController::setAllowZoom(bool allowZoom)
{
  d->allowZoom = allowZoom;
}

//################################################################################################
float FlatController::rotationAngle() const
{
  return d->rotationAngle;
}

//################################################################################################
void FlatController::setRotationAngle(float rotationAngle)
{
  d->rotationAngle = rotationAngle;
  update();
}

//################################################################################################
float FlatController::viewAngle() const
{
  return d->viewAngle;
}

//################################################################################################
void FlatController::setViewAngle(float viewAngle)
{
  d->viewAngle = viewAngle;
  update();
}

//##################################################################################################
float FlatController::rotationFactor() const
{
  return d->rotationFactor;
}

//##################################################################################################
void FlatController::setRotationFactor(float rotationFactor)
{
  d->rotationFactor = rotationFactor;
}

//##################################################################################################
void FlatController::assignMouseButtons(Button rotateButton, Button translateButton)
{
  d->rotateButton = rotateButton;
  d->translateButton = translateButton;
}

//##################################################################################################
void FlatController::saveState(nlohmann::json& j) const
{
  j["View angle"]     = d->viewAngle;
  j["Rotation angle"] = d->rotationAngle;
  j["Focal point"]    = tp_math_utils::vec3ToJSON(d->focalPoint);
  j["Distance"]       = d->distance;
}

//##################################################################################################
void FlatController::loadState(const nlohmann::json& j)
{
  d->viewAngle     = TPJSONFloat               (j, "View angle"    , d->viewAngle    );
  d->rotationAngle = TPJSONFloat               (j, "Rotation angle", d->rotationAngle);
  d->focalPoint    = tp_math_utils::getJSONVec3(j, "Focal point"   , d->focalPoint   );
  d->distance      = TPJSONFloat               (j, "Distance"      , d->distance     );

  update();
}

//##################################################################################################
FlatController::~FlatController()
{
  delete d;
}

//##################################################################################################
void FlatController::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
}

//##################################################################################################
void FlatController::updateMatrices()
{
  if(std::fabs(d->distance) < 0.000000001f)
    d->distance = 1.0f;

  //The width and height of the map widget
  float width  = float(map()->width());
  float height = float(map()->height());

  float fh = 1.0f;
  float fw = 1.0f;
  if(width>height)
    fw = width/height;
  else
    fh = height/width;

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::rotate(view, glm::radians(d->viewAngle), glm::vec3(1.0f, 0.0f, 0.0f));
  view = glm::rotate(view, glm::radians(d->rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
  view = glm::translate(view, -d->focalPoint);

  glm::mat4 projection = glm::ortho(-fw*d->distance,     // <- Left
                                    fw*d->distance,      // <- Right
                                    -fh*d->distance,     // <- Bottom
                                    fh*d->distance,      // <- Top
                                    -100.0f*d->distance, // <- Near
                                    100.0f*d->distance); // <- Far
  Matrices vp;
  vp.p  = projection;
  vp.v  = view;
  vp.vp = projection * view;
  setMatrices(defaultSID(), vp);
}

//##################################################################################################
bool FlatController::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
  case MouseEventType::DragStart: //----------------------------------------------------------------
  {
    translateInteractionStarted();

    if(d->mouseInteraction == Button::NoButton)
    {
      d->mouseInteraction = event.button;
      d->previousPos = event.pos;
    }

    return true;
  }

  case MouseEventType::Move: //---------------------------------------------------------------------
  {
    glm::ivec2 pos = event.pos;

    float dx = float(pos.x - d->previousPos.x);
    float dy = float(pos.y - d->previousPos.y);

    d->previousPos2 = d->previousPos;
    d->previousPos = pos;

    if(d->mouseInteraction == d->rotateButton)
    {
      if(d->variableViewAngle)
      {
        d->viewAngle += dy*0.2f;

        if(d->viewAngle>0)
          d->viewAngle = 0;

        if(d->viewAngle<-180)
          d->viewAngle = -180;
      }

      if(d->allowRotation)
      {
        d->rotationAngle += dx*d->rotationFactor;
        if(d->rotationAngle<0)
          d->rotationAngle+=360;

        if(d->rotationAngle>360)
          d->rotationAngle-=360;
      }
      update();
      return true;
    }
    else if(d->mouseInteraction == d->translateButton && d->allowTranslation)
    {
      translate(dx, dy, 1);
      update();
      return true;
    }

    break;
  }

  case MouseEventType::Release: //------------------------------------------------------------------
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

  case MouseEventType::Wheel: //--------------------------------------------------------------------
  {
    if(!d->allowZoom)
      return true;

    glm::vec3 scenePointA;
    bool moveOrigin = map()->unProject(event.pos, scenePointA, tp_math_utils::Plane());


    if(event.delta<0)
      d->distance *= 1.1f;
    else if(event.delta>0)
      d->distance *= 0.9f;
    else
      return true;

    if(moveOrigin)
    {
      updateMatrices();
      glm::vec3 scenePointB;
      moveOrigin = map()->unProject(event.pos, scenePointB, tp_math_utils::Plane());

      if(moveOrigin)
        d->focalPoint += scenePointA - scenePointB;
    }

    update();
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
  };

  return false;
}

//##################################################################################################
void FlatController::translate(float dx, float dy, double msSincePrevious)
{
  TP_UNUSED(msSincePrevious);
  d->translate(dx, dy);
}

//##################################################################################################
void FlatController::translateInteractionStarted()
{
}

//##################################################################################################
void FlatController::translateInteractionFinished()
{
}

}
