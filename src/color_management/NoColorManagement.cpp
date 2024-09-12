#include "tp_maps/color_management/NoColorManagement.h"

#include "tp_utils/Resources.h"

namespace tp_maps
{

//##################################################################################################
NoColorManagement::~NoColorManagement()=default;

//##################################################################################################
glm::vec4 NoColorManagement::toLinear(const glm::vec4& color) const
{
  return color;
}

//##################################################################################################
glm::vec3 NoColorManagement::toLinear(const glm::vec3& color) const
{
  return color;
}

//##################################################################################################
glm::vec4 NoColorManagement::fromLinear(const glm::vec4& color) const
{
  return color;
}

//##################################################################################################
glm::vec3 NoColorManagement::fromLinear(const glm::vec3& color) const
{
  return color;
}

//##################################################################################################
std::string NoColorManagement::glsl() const
{
  return tp_utils::resource("/tp_maps/NoColorManagement.glsl").data;
}

}
