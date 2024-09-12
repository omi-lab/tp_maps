#include "tp_maps/color_management/FilmicColorManagement.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/Resources.h"

namespace tp_maps
{

namespace
{
constexpr float gamma{2.2f};
}

//##################################################################################################
FilmicColorManagement::~FilmicColorManagement()=default;

//##################################################################################################
glm::vec4 FilmicColorManagement::toLinear(const glm::vec4& color) const
{
  return glm::vec4(glm::pow(glm::vec3(color), glm::vec3(gamma)), color.w);
}

//##################################################################################################
glm::vec3 FilmicColorManagement::toLinear(const glm::vec3& color) const
{
  glm::vec3 hsv = tp_math_utils::rgb2hsv(glm::pow(color, glm::vec3(gamma)));
  hsv.y /= 0.9f;
  return tp_math_utils::hsv2rgb(hsv);
}

//##################################################################################################
glm::vec4 FilmicColorManagement::fromLinear(const glm::vec4& color) const
{
  return glm::vec4(glm::pow(glm::vec3(color), glm::vec3(1.0f/gamma)), color.w);
}

//##################################################################################################
glm::vec3 FilmicColorManagement::fromLinear(const glm::vec3& color) const
{
  glm::vec3 hsv = tp_math_utils::rgb2hsv(color);
  hsv.y *= 0.9f;
  return glm::pow(tp_math_utils::hsv2rgb(hsv), glm::vec3(1.0f/gamma));
}

//##################################################################################################
std::string FilmicColorManagement::glsl() const
{
  return tp_utils::resource("/tp_maps/FilmicColorManagement.glsl").data;
}

}
