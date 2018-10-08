#include "tp_maps/controllers/FlatController.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/transform.hpp"

namespace tp_maps
{
//##################################################################################################
struct FlatController::Private
{
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
  bool mouseMoved{false};

  //################################################################################################
  Private(FlatController* q_):
    q(q_)
  {

  }

  //################################################################################################
  void translate(float dx, float dy)
  {
    float radians = glm::radians(rotationAngle);
    float ca = cos(radians);
    float sa = sin(radians);

    //The width and height of the map widget
    float width(q->map()->width());
    float height(q->map()->height());

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
glm::vec3 FlatController::focalPoint()const
{
  return d->focalPoint;
}

//##################################################################################################
void FlatController::setFocalPoint(const glm::vec3& focalPoint)
{
  d->focalPoint = focalPoint;
  map()->update();
}

//##################################################################################################
float FlatController::distance()const
{
  return d->distance;
}

//##################################################################################################
void FlatController::setDistance(float distance)
{
  d->distance = distance;
  map()->update();
}

//##################################################################################################
bool FlatController::allowRotation()const
{
  return d->allowRotation;
}

//##################################################################################################
void FlatController::setAllowRotation(bool allowRotation)
{
  d->allowRotation = allowRotation;
}

//##################################################################################################
bool FlatController::variableViewAngle()const
{
  return d->variableViewAngle;
}

//##################################################################################################
void FlatController::setVariableViewAngle(bool variableViewAngle)
{
  d->variableViewAngle = variableViewAngle;
}

//##################################################################################################
bool FlatController::allowTranslation()const
{
  return d->allowTranslation;
}

//##################################################################################################
void FlatController::setAllowTranslation(bool allowTranslation)
{
  d->allowTranslation = allowTranslation;
}

//##################################################################################################
bool FlatController::allowZoom()const
{
  return d->allowZoom;
}

//##################################################################################################
void FlatController::setAllowZoom(bool allowZoom)
{
  d->allowZoom = allowZoom;
}

//################################################################################################
float FlatController::rotationAngle()const
{
  return d->rotationAngle;
}

//################################################################################################
void FlatController::setRotationAngle(float rotationAngle)
{
  d->rotationAngle = rotationAngle;
  map()->update();
}

//##################################################################################################
float FlatController::rotationFactor()const
{
  return d->rotationFactor;
}

//##################################################################################################
void FlatController::setRotationFactor(float rotationFactor)
{
  d->rotationFactor = rotationFactor;
}

//##################################################################################################
nlohmann::json FlatController::saveState()const
{
  nlohmann::json j;

  j["View angle"]     = d->viewAngle;
  j["Rotation angle"] = d->rotationAngle;
  j["Focal point"]    = tp_math_utils::vec3ToJSON(d->focalPoint);
  j["Distance"]       = d->distance;

  return j;
}

//##################################################################################################
void FlatController::loadState(const nlohmann::json& j)
{
  d->viewAngle     = tp_utils::getJSONValue<float>(j, "View angle"    , d->viewAngle    );
  d->rotationAngle = tp_utils::getJSONValue<float>(j, "Rotation angle", d->rotationAngle);
  d->focalPoint    = tp_math_utils::getJSONVec3   (j, "Focal point"   , d->focalPoint   );
  d->distance      = tp_utils::getJSONValue<float>(j, "Distance"      , d->distance     );

  map()->update();
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
  if(fabs(d->distance) < 0.000000001f)
    d->distance = 1.0f;

  //The width and height of the map widget
  float width(map()->width());
  float height(map()->height());

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

  setMatrix(defaultSID(), projection * view);
}

//##################################################################################################
bool FlatController::mouseEvent(const MouseEvent& event)
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

    float dx = pos.x - d->previousPos.x;
    float dy = pos.y - d->previousPos.y;

    d->previousPos2 = d->previousPos;
    d->previousPos = pos;

    if(d->mouseInteraction == Button::RightButton)
    {
      if(d->variableViewAngle)
      {
        d->viewAngle += dy*0.2;

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
  };

  return true;
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
