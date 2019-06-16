#include "tp_maps/layers/PlaneLayer.h"

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/AngleBetweenVectors.h"

#include "glm/gtx/transform.hpp"

namespace tp_maps
{

//##################################################################################################
struct PlaneLayer::Private
{
  PlaneLayer* q;
  tp_math_utils::Plane plane;
  glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};

  //################################################################################################
  Private(PlaneLayer* q_):
    q(q_)
  {

  }

  //################################################################################################
  void updatePlane()
  {
    const glm::vec3 o = plane.pointAndNormal()[0];
    const glm::vec3 x = plane.threePoints()[1] - o;
    const glm::vec3 y = plane.threePoints()[2] - o;
    const glm::vec3 z = plane.pointAndNormal()[1];

    auto proj = [&](const glm::vec3& coord)
    {
      return (coord.x*x) + (coord.y*y) + (coord.z*z) + o;
    };

    Lines lines;
    lines.color = color;
    lines.mode = GL_LINES;

    auto addLine = [&](const glm::vec3& a, const glm::vec3& b)
    {
      lines.lines.push_back(proj(a));
      lines.lines.push_back(proj(b));
    };

    int size=10;
    auto sizeF = float(size);

    //The center cross
    addLine({0.0f, -sizeF, 0.0f}, {0.0f, sizeF, 0.0f});
    addLine({-sizeF, 0.0f, 0.0f}, {sizeF, 0.0f, 0.0f});

    //Lines either side of the cross
    for(int i=1; i<=size; i++)
    {
      addLine({-float(i), -sizeF, 0.0f}, {-float(i), sizeF, 0.0f});
      addLine({ float(i), -sizeF, 0.0f}, { float(i), sizeF, 0.0f});

      addLine({-sizeF, -float(i), 0.0f}, {sizeF, -float(i), 0.0f});
      addLine({-sizeF,  float(i), 0.0f}, {sizeF,  float(i), 0.0f});
    }

    q->setLines({lines});
  }
};

//##################################################################################################
PlaneLayer::PlaneLayer():
  d(new Private(this))
{

}

//##################################################################################################
PlaneLayer::~PlaneLayer()
{
  delete d;
}

//##################################################################################################
const tp_math_utils::Plane& PlaneLayer::plane()const
{
  return d->plane;
}

//##################################################################################################
void PlaneLayer::setPlane(const tp_math_utils::Plane& plane)
{
  d->plane = plane;
  d->updatePlane();
}

//##################################################################################################
const glm::vec4& PlaneLayer::color()const
{
  return d->color;
}

//##################################################################################################
void PlaneLayer::setColor(const glm::vec4& color)
{
  d->color = color;
  d->updatePlane();
}

}
