#include "tp_maps/controllers/AnimatedFlatController.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/JSONUtils.h"
#include "tp_math_utils/Plane.h"

#include "tp_utils/JSONUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/norm.hpp>

namespace tp_maps
{
//##################################################################################################
struct AnimatedFlatController::Private
{
  AnimatedFlatController* q;

  float distance{10.0f};
  float rotationAngle{0.0f};
  glm::vec3 focalPoint{0, 0, 0};

  bool animate{false};

  double timestampMS{0.0};
  glm::vec2 inertia{0.0f, 0.0f};

  //################################################################################################
  Private(AnimatedFlatController* q_):
    q(q_)
  {

  }

  //################################################################################################
  void updateInertia()
  {
    //inertia.x = inertia.x * 0.9f;
    //inertia.y = inertia.y * 0.9f;
    //
    //if((fabs(inertia.x) + fabs(inertia.y)) < 0.0001f)
    //  return;
    //
    ////float f = 1.5f;
    ////translate(inertia.x*f, inertia.y*f);
    //
    //q->map()->update();
  }
};

//##################################################################################################
AnimatedFlatController::AnimatedFlatController(Map* map):
  FlatController(map),
  d(new Private(this))
{

}

//################################################################################################
void AnimatedFlatController::animationReset()
{
  d->focalPoint    = focalPoint();
  d->distance      = distance();
  d->rotationAngle = rotationAngle();
  d->timestampMS   = 0.0;
  d->animate       = false;
}

//##################################################################################################
void AnimatedFlatController::animateToFocalPoint(const glm::vec3& focalPoint)
{
  d->focalPoint = focalPoint;
  d->animate = true;
}

//##################################################################################################
void AnimatedFlatController::animateToDistance(float distance)
{
  d->distance = distance;
  d->animate = true;
}

//##################################################################################################
void AnimatedFlatController::animateToRotationAngle(float rotationAngle)
{
  d->rotationAngle = rotationAngle;
  d->animate = true;
}

//##################################################################################################
AnimatedFlatController::~AnimatedFlatController()
{
  delete d;
}

//##################################################################################################
bool AnimatedFlatController::mouseEvent(const MouseEvent& event)
{
  switch(event.type)
  {
  case MouseEventType::Press:       d->animate = false; break;
  case MouseEventType::Release:     d->animate = false; break;
  case MouseEventType::Wheel:       d->animate = false; break;
  case MouseEventType::DoubleClick: d->animate = false; break;

  default: break;
  };

  return FlatController::mouseEvent(event);
}

//##################################################################################################
void AnimatedFlatController::animate(double timestampMS)
{
  if(!d->animate || d->timestampMS < (timestampMS-4000.0))
  {
    d->timestampMS = timestampMS;
    return;
  }

  d->updateInertia();

  float f = ((timestampMS - d->timestampMS)/1000.0);

  glm::vec3 focalPointD    = (d->focalPoint    - focalPoint()   );
  float     distanceD      = (d->distance      - distance()     );
  float     rotationAngleD = (d->rotationAngle - rotationAngle());

  if(glm::length2(focalPointD) < 0.0001f &&
     fabs(distanceD)           < 0.0001f &&
     fabs(rotationAngleD)      < 0.0001f)
  {
    d->animate = false;
    d->timestampMS=0.0;
    return;
  }

  focalPointD    *= f;
  distanceD      *= f;
  rotationAngleD *= f;

  setFocalPoint   (focalPoint()    + focalPointD   );
  setDistance     (distance()      + distanceD     );
  setRotationAngle(rotationAngle() + rotationAngleD);
}

//##################################################################################################
void AnimatedFlatController::translate(float dx, float dy, double msSincePrevious)
{
  //Save the most recent translation here so that we can calculate inertia later.
  FlatController::translate(dx, dy, msSincePrevious);
}

//##################################################################################################
void AnimatedFlatController::translateInteractionStarted()
{
  d->inertia = {0.0f, 0.0f};
}

//##################################################################################################
void AnimatedFlatController::translateInteractionFinished()
{
  //Set inertia here


  //glm::ivec2 delta = event.pos - d->previousPos2;
  //d->inertia.x = delta.x;
  //d->inertia.y = delta.y;

}

}
