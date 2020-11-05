#include "tp_maps/controllers/CADController.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"
#include "tp_utils/DebugUtils.h"

#include "glm/gtx/transform.hpp"

namespace tp_maps
{

//##################################################################################################
std::vector<std::string> cadControllerModes()
{
  return
  {
    "Perspective",
    "OrthoXZ",
    "OrthoYZ",
    "OrthoXY"
  };
}

//##################################################################################################
std::string cadControllerModeToString(CADControllerMode mode)
{
  switch(mode)
  {
  case CADControllerMode::Perspective: return "Perspective";
  case CADControllerMode::OrthoXZ:     return "OrthoXZ";
  case CADControllerMode::OrthoYZ:     return "OrthoYZ";
  case CADControllerMode::OrthoXY:     return "OrthoXY";
  }

  return "Perspective";
}

//##################################################################################################
CADControllerMode cadControllerModeFromString(std::string mode)
{
  if(mode == "Perspective")return CADControllerMode::Perspective;
  if(mode == "OrthoXZ")    return CADControllerMode::OrthoXZ;
  if(mode == "OrthoYZ")    return CADControllerMode::OrthoYZ;
  if(mode == "OrthoXY")    return CADControllerMode::OrthoXY;
  return CADControllerMode::Perspective;
}

//##################################################################################################
struct CADController::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::CADController::Private");
  TP_NONCOPYABLE(Private);

  CADController* q;

  float speedModifier{1.0f};

  CADControllerMode mode{CADControllerMode::Perspective};

  glm::ivec2 previousPos{0,0};
  glm::ivec2 previousPos2{0,0};
  Button mouseInteraction{Button::NoButton};

  std::map<int32_t, bool> keyState;

  double timestampMS{0.0};

  float viewAngle{-90.0f};   //!< In degrees.
  float rotationAngle{0.0f}; //!< In degrees.
  glm::vec3 cameraOrigin{0, 0, 1.8f};
  float rotationFactor{0.2f};

  float distance{10.0f};     //!< Distance for ortho projections.
  glm::vec3 focalPoint{0.0f,0.0f,0.0f};

  float near{0.01f};
  float far{100.0f};

  bool mouseMoved{false};

  //Behaviour
  bool allowRotation{true};
  bool variableViewAngle{true};
  bool fullScreen{true};

  //################################################################################################
  Private(CADController* q_, bool fullScreen_):
    q(q_),
    fullScreen(fullScreen_)
  {
    keyState[UP_KEY   ]      = false;
    keyState[LEFT_KEY ]      = false;
    keyState[RIGHT_KEY]      = false;
    keyState[DOWN_KEY ]      = false;

    keyState[W_KEY]          = false;
    keyState[A_KEY]          = false;
    keyState[S_KEY]          = false;
    keyState[D_KEY]          = false;

    keyState[SPACE_KEY]      = false;

    keyState[L_SHIFT_KEY]    = false;
    keyState[R_SHIFT_KEY]    = false;

    keyState[L_CTRL_KEY]     = false;

    keyState[PAGE_UP_KEY ]   = false;
    keyState[PAGE_DOWN_KEY ] = false;
  }

  //################################################################################################
  void translate(float dist)
  {
    float radians = glm::radians(rotationAngle);
    float ca = std::cos(radians);
    float sa = std::sin(radians);
    cameraOrigin.x += (dist*sa) * speedModifier;
    cameraOrigin.y += (dist*ca) * speedModifier;
  }

  //################################################################################################
  void strafe(float distX, float distY=0.0f)
  {
    float radians = glm::radians(rotationAngle);
    float ca = std::cos(radians);
    float sa = std::sin(radians);

    glm::vec3 forward{sa, ca, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec right = glm::cross(forward, up);

    cameraOrigin.x += (right.x*distX)*speedModifier;
    cameraOrigin.y += (right.y*distX)*speedModifier;

    glm::mat4 m = glm::inverse(q->matrix(defaultSID()));
    glm::vec4 v1 = m*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 v2 = m*glm::vec4(0.0f, -1.0f, 0.0f, 1.0f);
    cameraOrigin += (glm::normalize((glm::vec3(v2)/v2.w) - (glm::vec3(v1)/v1.w)) * distY) * speedModifier;
  }

  //################################################################################################
  void translate(float dx, float dy)
  {
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

    dx *= (fw*distance*2.0f) * speedModifier;
    dy *= (fh*distance*2.0f) * speedModifier;

    switch(mode)
    {
    case CADControllerMode::OrthoXZ:
      focalPoint.x -= dx;
      focalPoint.z += dy;
      break;

    case CADControllerMode::OrthoYZ:
      focalPoint.y -= dx;
      focalPoint.z += dy;
      break;

    case CADControllerMode::OrthoXY:
      focalPoint.x -= dx;
      focalPoint.y += dy;
      break;

    default:
      break;
    }
  }

  //################################################################################################
  void moveForward(float distZ)
  {
    glm::mat4 m = glm::inverse(q->matrix(defaultSID()));
    glm::vec4 v1 = m*glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    glm::vec4 v2 = m*glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    cameraOrigin += (glm::normalize((glm::vec3(v2)/v2.w) - (glm::vec3(v1)/v1.w)) * distZ) * speedModifier;
  }
};

//##################################################################################################
CADController::CADController(Map* map, bool fullScreen):
  Controller(map),
  d(new Private(this, fullScreen))
{

}

//##################################################################################################
CADControllerMode CADController::mode() const
{
  return d->mode;
}

//##################################################################################################
void CADController::setMode(CADControllerMode mode)
{
  d->mode = mode;

  switch(d->mode)
  {
  case CADControllerMode::OrthoXY:
  case CADControllerMode::OrthoXZ:
  case CADControllerMode::OrthoYZ:
    d->focalPoint = {0.0f, 0.0f, 0.0f};
    break;

  default:
    break;
  }

  map()->update();
}

//##################################################################################################
void CADController::setSpeedModifier(float speedModifier)
{
  d->speedModifier = speedModifier;
}

//##################################################################################################
glm::vec3 CADController::cameraOrigin() const
{
  return d->cameraOrigin;
}

//##################################################################################################
void CADController::setCameraOrigin(const glm::vec3& cameraOrigin)
{
  d->cameraOrigin = cameraOrigin;
  map()->update();
}

//##################################################################################################
bool CADController::allowRotation() const
{
  return d->allowRotation;
}

//##################################################################################################
void CADController::setAllowRotation(bool allowRotation)
{
  d->allowRotation = allowRotation;
}

//##################################################################################################
bool CADController::variableViewAngle() const
{
  return d->variableViewAngle;
}

//##################################################################################################
void CADController::setVariableViewAngle(bool variableViewAngle)
{
  d->variableViewAngle = variableViewAngle;
}

//################################################################################################
float CADController::rotationAngle() const
{
  return d->rotationAngle;
}

//################################################################################################
void CADController::setRotationAngle(float rotationAngle)
{
  d->rotationAngle = rotationAngle;
  map()->update();
}

//##################################################################################################
float CADController::rotationFactor() const
{
  return d->rotationFactor;
}

//##################################################################################################
void CADController::setRotationFactor(float rotationFactor)
{
  d->rotationFactor = rotationFactor;
}

//##################################################################################################
void CADController::setNearAndFar(float near, float far)
{
  d->near = near;
  d->far = far;
  map()->update();
}

//##################################################################################################
glm::vec3 CADController::forward() const
{
  glm::mat4 m = glm::inverse(matrix(defaultSID()));
  glm::vec4 o=m*glm::vec4(0,0,0,1);
  glm::vec4 f=m*glm::vec4(0,0,1,1);
  return glm::normalize((glm::vec3(f)/f.w) - (glm::vec3(o)/o.w));
}

//##################################################################################################
glm::vec3 CADController::up() const
{
  glm::mat4 m = glm::inverse(matrix(defaultSID()));
  glm::vec4 o=m*glm::vec4(0,0,0,1);
  glm::vec4 u=m*glm::vec4(0,1,0,1);
  return glm::normalize((glm::vec3(u)/u.w) - (glm::vec3(o)/o.w));
}

//##################################################################################################
void CADController::setOrientation(const glm::vec3& forward, const glm::vec3& up)
{
  // Calculate the rotation first
  {
    glm::vec2 direction;

    float dotUp = glm::dot(glm::vec3(0,0,1), up);
    float dotForward = glm::dot(glm::vec3(0,0,1), forward);

    if(std::fabs(dotForward) < std::fabs(dotUp))
      direction = forward;
    else
      direction = glm::vec2(up);

    direction = glm::normalize(direction);

    d->rotationAngle = glm::degrees(std::atan2(direction.x,direction.y));
    if(d->rotationAngle<0.0f)
      d->rotationAngle += 360.0f;
  }


  // Then the view angle
  {
    d->viewAngle = glm::degrees(std::acos(forward.z))-180.0f;
  }

  map()->update();
}

//##################################################################################################
nlohmann::json CADController::saveState() const
{
  nlohmann::json j;

  j["View angle"]     = d->viewAngle;
  j["Rotation angle"] = d->rotationAngle;
  j["Camera origin"]  = tp_math_utils::vec3ToJSON(d->cameraOrigin);
  j["Focal point"]    = tp_math_utils::vec3ToJSON(d->focalPoint);
  j["Distance"]       = d->distance;
  j["Mode"]           = cadControllerModeToString(d->mode);

  return j;
}

//##################################################################################################
void CADController::loadState(const nlohmann::json& j)
{
  d->viewAngle     =                             TPJSONFloat (j, "View angle"    , d->viewAngle                       );
  d->rotationAngle =                             TPJSONFloat (j, "Rotation angle", d->rotationAngle                   );
  d->cameraOrigin  =               tp_math_utils::getJSONVec3(j, "Camera origin" , d->cameraOrigin                    );
  d->focalPoint    =               tp_math_utils::getJSONVec3(j, "Focal point"   , d->focalPoint                      );
  d->distance      =                             TPJSONFloat (j, "Distance"      , d->distance                        );
  d->mode          = cadControllerModeFromString(TPJSONString(j, "Mode"          , cadControllerModeToString(d->mode)));
  map()->update();
}


//################################################################################################
void CADController::copyState(const CADController& other)
{
  d->viewAngle      = other.d->viewAngle;
  d->rotationAngle  = other.d->rotationAngle;
  d->cameraOrigin   = other.d->cameraOrigin;
  d->rotationFactor = other.d->rotationFactor;

  d->distance       = other.d->distance;
  d->focalPoint     = other.d->focalPoint;

  d->near           = other.d->near;
  d->far            = other.d->far;

  map()->update();
}

//##################################################################################################
CADController::~CADController()
{
  delete d;
}

//##################################################################################################
void CADController::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
}

//##################################################################################################
void CADController::updateMatrices()
{
  //The width and height of the map widget
  float width  = float(map()->width());
  float height = float(map()->height());

  if(width<1.0f || height<1.0f)
    return;

  float fh = 1.0f;
  float fw = 1.0f;
  if(width>height)
    fw = width/height;
  else
    fh = height/width;

  glm::mat4 view{1.0f};
  glm::mat4 projection;

  auto makeOrtho = [&]
  {
    projection = glm::ortho(-fw*d->distance,     // <- Left
                            fw*d->distance,      // <- Right
                            -fh*d->distance,     // <- Bottom
                            fh*d->distance,      // <- Top
                            -100.0f*d->distance, // <- Near
                            100.0f*d->distance); // <- Far

  };

  switch(d->mode)
  {
  case CADControllerMode::Perspective:
  {
    view = glm::mat4(1.0f);
    view = glm::rotate(view, glm::radians(d->viewAngle), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(d->rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::translate(view, -d->cameraOrigin);

    projection = glm::perspective(glm::radians(63.0f), fw/fh, d->near, d->far);
    break;
  }

  case CADControllerMode::OrthoXZ:
  {
    view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, -d->focalPoint*glm::vec3(1.0f, 0.0f, 1.0f));
    makeOrtho();
    break;
  }

  case CADControllerMode::OrthoYZ:
  {
    view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    view = glm::translate(view, -d->focalPoint*glm::vec3(0.0f, 1.0f, 1.0f));
    makeOrtho();
    break;
  }

  case CADControllerMode::OrthoXY:
  {
    view = glm::translate(view, -d->focalPoint*glm::vec3(1.0f, 1.0f, 0.0f));
    makeOrtho();
    break;
  }
  }

  Matrices vp;
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
bool CADController::mouseEvent(const MouseEvent& event)
{
  const int mouseSensitivity=8;
  constexpr float metersPerSecond = 5.6f;
  constexpr float translationFactor = metersPerSecond / 1000.0f;

  bool changed=false;
  TP_CLEANUP([&]
  {
    if(changed)
      userInteraction();
  });

  switch(event.type)
  {
  case MouseEventType::Press: //--------------------------------------------------------------------
  {
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

    float dx{0.0f};
    float dy{0.0f};

    if(d->fullScreen)
    {
      dx = float(event.posDelta.x);
      dy = float(event.posDelta.y);
    }
    else
    {
      dx = float(pos.x - d->previousPos.x);
      dy = float(pos.y - d->previousPos.y);
    }

    d->previousPos2 = d->previousPos;
    d->previousPos = pos;

    if(d->mouseInteraction == Button::MiddleButton)
    {
      changed = true;
      if(d->mode == CADControllerMode::Perspective)
        d->strafe(dx*float(translationFactor), dy*float(translationFactor));
      else
        d->translate(dx, dy);
      map()->update();
    }
    else if(d->fullScreen || d->mouseInteraction == Button::RightButton)
    {
      if(d->mode == CADControllerMode::Perspective)
      {
        if(d->variableViewAngle)
        {
          changed = true;
          d->viewAngle += dy*0.2f;

          if(d->viewAngle>0)
            d->viewAngle = 0;

          if(d->viewAngle<-180)
            d->viewAngle = -180;
        }

        if(d->allowRotation)
        {
          changed = true;
          d->rotationAngle += dx*d->rotationFactor;
          if(d->rotationAngle<0)
            d->rotationAngle+=360;

          if(d->rotationAngle>360)
            d->rotationAngle-=360;
        }
      }
      map()->update();
    }

    break;
  }

  case MouseEventType::Wheel: //--------------------------------------------------------------------
  {
    if(d->mode == CADControllerMode::Perspective)
    {
      changed = true;
      d->moveForward(float(event.delta)*translationFactor*0.2f);
    }
    else
    {
      changed = true;
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
    }

    map()->update();
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
    }
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
bool CADController::keyEvent(const KeyEvent& event)
{
  //tpWarning() << event.scancode;
  auto i = d->keyState.find(event.scancode);
  if(i == d->keyState.end())
    return true;

  switch(event.type)
  {
  case KeyEventType::Press: //----------------------------------------------------------------------
  {
    i->second = true;
    break;
  }

  case KeyEventType::Release: //--------------------------------------------------------------------
  {
    i->second = false;
    break;
  }
  }

  return true;
}

//##################################################################################################
void CADController::animate(double timestampMS)
{
  constexpr double degreesPerSecond = 100.0;
  constexpr double rotationFactor = degreesPerSecond / 1000.0;

  constexpr double metersPerSecond = 1.6;
  constexpr double translationFactor = metersPerSecond / 1000.0;

  if(d->timestampMS<1.0)
  {
    d->timestampMS = timestampMS;
    return;
  }

  bool changed=false;
  TP_CLEANUP([&]
  {
    if(changed)
      userInteraction();
  });

  double delta = timestampMS - d->timestampMS;
  d->timestampMS = timestampMS;

  double rotateDegrees   = rotationFactor * delta;
  double translateMeters = translationFactor * delta;

  //-- Perspective mode ----------------------------------------------------------------------------
  if(d->mode == CADControllerMode::Perspective)
  {
    if(d->keyState[L_SHIFT_KEY] ||d->keyState[R_SHIFT_KEY] )
    {
      changed = true;
      translateMeters *= 10;
    }

    if(d->keyState[UP_KEY] ||d->keyState[W_KEY] )
    {
      changed = true;
      d->translate(float(translateMeters));
      map()->update();
    }

    if(d->keyState[DOWN_KEY]||d->keyState[S_KEY] )
    {
      changed = true;
      d->translate(-float(translateMeters));
      map()->update();
    }

    if(d->keyState[LEFT_KEY])
    {
      changed = true;
      d->rotationAngle -= float(rotateDegrees);
      if(d->rotationAngle<0.0f)
        d->rotationAngle+=360.0f;
      map()->update();
    }

    if(d->keyState[RIGHT_KEY])
    {
      changed = true;
      d->rotationAngle += float(rotateDegrees);
      if(d->rotationAngle>360.0f)
        d->rotationAngle-=360.0f;
      map()->update();
    }

    if(d->keyState[A_KEY] )
    {
      changed = true;
      d->strafe(-float(translateMeters));
      map()->update();
    }

    if(d->keyState[D_KEY] )
    {
      changed = true;
      d->strafe(float(translateMeters));
      map()->update();
    }

    if(d->keyState[PAGE_UP_KEY] || d->keyState[SPACE_KEY])
    {
      changed = true;
      d->cameraOrigin.z += float(translateMeters) * d->speedModifier;
      map()->update();
    }

    if(d->keyState[PAGE_DOWN_KEY]|| d->keyState[L_CTRL_KEY])
    {
      changed = true;
      d->cameraOrigin.z -= float(translateMeters) * d->speedModifier;
      map()->update();
    }
  }
}

}
