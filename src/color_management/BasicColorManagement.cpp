#include "tp_maps/color_management/BasicColorManagement.h"

#include "tp_utils/Resources.h"

namespace tp_maps
{

namespace
{
constexpr float gamma{2.2f};
}

//##################################################################################################
BasicColorManagement::~BasicColorManagement()=default;

//##################################################################################################
glm::vec4 BasicColorManagement::toLinear(const glm::vec4& color) const
{
  return {toLinear(glm::vec3(color)), color.w};
}

//##################################################################################################
glm::vec3 BasicColorManagement::toLinear(const glm::vec3& color) const
{
  return glm::pow(color, glm::vec3(gamma));
}

//##################################################################################################
glm::vec4 BasicColorManagement::fromLinear(const glm::vec4& color) const
{
  return {fromLinear(glm::vec3(color)), color.w};
}

//##################################################################################################
glm::vec3 BasicColorManagement::fromLinear(const glm::vec3& color) const
{
  return glm::pow(color, glm::vec3(1.0f/gamma));
}

//##################################################################################################
std::string BasicColorManagement::glsl() const
{
  return tp_utils::resource("/tp_maps/BasicColorManagement.glsl").data;
}

}
