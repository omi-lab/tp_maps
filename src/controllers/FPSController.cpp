#include "tp_maps/controllers/FPSController.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"

#include "tp_utils/JSONUtils.h"

#include "glm/gtx/transform.hpp" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
struct FPSController::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FPSController::Private");
  TP_NONCOPYABLE(Private);

  FPSController* q;

  glm::ivec2 previousPos{0,0};
  glm::ivec2 previousPos2{0,0};
  Button mouseInteraction{Button::NoButton};

  std::map<int32_t, bool> keyState;

  double timestampMS{0.0};

  float viewAngle{-90.0f};
  float rotationAngle{0.0f};
  glm::vec3 cameraOrigin{0, 0, 1.8f};
  float rotationFactor{0.2f};

  float near{0.01f};
  float far{100.0f};

  //Behaviour
  bool allowRotation{true};
  bool variableViewAngle{true};
  bool fullScreen{true};

  //################################################################################################
  Private(FPSController* q_, bool fullScreen_):
    q(q_),
    fullScreen(fullScreen_)
  {
    keyState[TP_UP_KEY   ]      = false;
    keyState[TP_LEFT_KEY ]      = false;
    keyState[TP_RIGHT_KEY]      = false;
    keyState[TP_DOWN_KEY ]      = false;

    keyState[TP_W_KEY]          = false;
    keyState[TP_A_KEY]          = false;
    keyState[TP_S_KEY]          = false;
    keyState[TP_D_KEY]          = false;

    keyState[TP_SPACE_KEY]      = false;

    keyState[TP_L_SHIFT_KEY]    = false;
    keyState[TP_R_SHIFT_KEY]    = false;

    keyState[TP_PAGE_UP_KEY ]   = false;
    keyState[TP_PAGE_DOWN_KEY ] = false;
  }

  //################################################################################################
  void translate(float dist)
  {
    float radians = glm::radians(rotationAngle);
    float ca = std::cos(radians);
    float sa = std::sin(radians);
    cameraOrigin.x += dist*sa;
    cameraOrigin.y += dist*ca;
  }

  //################################################################################################
  void strafe(float dist)
  {
    float radians = glm::radians(rotationAngle);
    float ca = std::cos(radians);
    float sa = std::sin(radians);

    glm::vec3 forward{sa, ca, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec crossProd = glm::cross(forward, up);

    cameraOrigin.x += (crossProd.x*dist);
    cameraOrigin.y += (crossProd.y*dist);
  }
};

//##################################################################################################
FPSController::FPSController(Map* map, bool fullScreen):
  Controller(map),
  d(new Private(this, fullScreen))
{

}

//##################################################################################################
glm::vec3 FPSController::cameraOrigin() const
{
  return d->cameraOrigin;
}

//##################################################################################################
void FPSController::setCameraOrigin(const glm::vec3& cameraOrigin)
{
  d->cameraOrigin = cameraOrigin;
  update();
}

//##################################################################################################
bool FPSController::allowRotation() const
{
  return d->allowRotation;
}

//##################################################################################################
void FPSController::setAllowRotation(bool allowRotation)
{
  d->allowRotation = allowRotation;
}

//##################################################################################################
bool FPSController::variableViewAngle() const
{
  return d->variableViewAngle;
}

//##################################################################################################
void FPSController::setVariableViewAngle(bool variableViewAngle)
{
  d->variableViewAngle = variableViewAngle;
}

//################################################################################################
float FPSController::rotationAngle() const
{
  return d->rotationAngle;
}

//################################################################################################
void FPSController::setRotationAngle(float rotationAngle)
{
  d->rotationAngle = rotationAngle;
  update();
}

//##################################################################################################
float FPSController::rotationFactor() const
{
  return d->rotationFactor;
}

//##################################################################################################
void FPSController::setRotationFactor(float rotationFactor)
{
  d->rotationFactor = rotationFactor;
}

//##################################################################################################
void FPSController::setNearAndFar(float near, float far)
{
  d->near = near;
  d->far = far;
  update();
}

//##################################################################################################
void FPSController::saveState(nlohmann::json& j) const
{
  j["View angle"]     = d->viewAngle;
  j["Rotation angle"] = d->rotationAngle;
  j["Camera origin"]    = tp_math_utils::vec3ToJSON(d->cameraOrigin);
}

//##################################################################################################
void FPSController::loadState(const nlohmann::json& j)
{
  d->viewAngle     = TPJSONFloat               (j, "View angle"    , d->viewAngle    );
  d->rotationAngle = TPJSONFloat               (j, "Rotation angle", d->rotationAngle);
  d->cameraOrigin  = tp_math_utils::getJSONVec3(j, "Camera origin" , d->cameraOrigin );

  update();
}

//##################################################################################################
FPSController::~FPSController()
{
  delete d;
}

//##################################################################################################
void FPSController::mapResized(int w, int h)
{
  TP_UNUSED(w);
  TP_UNUSED(h);
}

//##################################################################################################
void FPSController::updateMatrices()
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

  glm::mat4 view = glm::mat4(1.0f);
  view = glm::rotate(view, glm::radians(d->viewAngle), glm::vec3(1.0f, 0.0f, 0.0f));
  view = glm::rotate(view, glm::radians(d->rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
  view = glm::translate(view, -d->cameraOrigin);

  glm::mat4 projection = glm::perspective(glm::radians(63.0f), fw/fh, d->near, d->far);

  Matrices vp;
  vp.p  = projection;
  vp.v  = view;
  vp.vp = projection * view;
  setMatrices(defaultSID(), vp);
}

//##################################################################################################
bool FPSController::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
  case MouseEventType::Press: //--------------------------------------------------------------------
  {
    if(d->mouseInteraction == Button::NoButton)
    {
      d->mouseInteraction = event.button;
      d->previousPos = event.pos;
      return true;
    }
    break;
  }

  case MouseEventType::Move: //---------------------------------------------------------------------
  {
    glm::ivec2 pos = event.pos;

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

    if(d->fullScreen || d->mouseInteraction == Button::RightButton)
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

    break;
  }

  case MouseEventType::Release: //------------------------------------------------------------------
  {
    if(event.button == d->mouseInteraction)
    {
      d->mouseInteraction = Button::NoButton;
      return true;
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
  }

  return false;
}

//##################################################################################################
bool FPSController::keyEvent(const KeyEvent& event)
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
void FPSController::animate(double timestampMS)
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

  double delta = timestampMS - d->timestampMS;
  d->timestampMS = timestampMS;

  double rotateDegrees   = rotationFactor * delta;
  double translateMeters = translationFactor * delta;

  if(d->keyState[TP_L_SHIFT_KEY] ||d->keyState[TP_R_SHIFT_KEY] )
  {
    translateMeters *= 10;
  }

  if(d->keyState[TP_UP_KEY] ||d->keyState[TP_W_KEY] )
  {
    d->translate(float(translateMeters));
    update();
  }

  if(d->keyState[TP_DOWN_KEY]||d->keyState[TP_S_KEY] )
  {
    d->translate(-float(translateMeters));
    update();
  }

  if(d->keyState[TP_LEFT_KEY])
  {
    d->rotationAngle -= float(rotateDegrees);
    if(d->rotationAngle<0.0f)
      d->rotationAngle+=360.0f;
    update();
  }

  if(d->keyState[TP_RIGHT_KEY])
  {
    d->rotationAngle += float(rotateDegrees);
    if(d->rotationAngle>360.0f)
      d->rotationAngle-=360.0f;
    update();
  }

  if(d->keyState[TP_A_KEY] )
  {
    d->strafe(-float(translateMeters));
    update();
  }

  if(d->keyState[TP_D_KEY] )
  {
    d->strafe(float(translateMeters));
    update();
  }

  if(d->keyState[TP_PAGE_UP_KEY] || d->keyState[TP_SPACE_KEY])
  {
    d->cameraOrigin.z += float(translateMeters);
    update();
  }

  if(d->keyState[TP_PAGE_DOWN_KEY])
  {
    d->cameraOrigin.z -= float(translateMeters);
    update();
  }
}

}
