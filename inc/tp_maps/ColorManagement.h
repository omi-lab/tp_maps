#ifndef tp_maps_ColorManagement_h
#define tp_maps_ColorManagement_h

#include "tp_maps/Globals.h"

namespace tp_maps
{

//##################################################################################################
class TP_MAPS_EXPORT ColorManagement
{
  TP_NONCOPYABLE(ColorManagement);
public:
  //################################################################################################
  ColorManagement();

  //################################################################################################
  virtual ~ColorManagement();

  //################################################################################################
  virtual glm::vec4 toLinear(const glm::vec4& color) const=0;

  //################################################################################################
  virtual glm::vec3 toLinear(const glm::vec3& color) const=0;

  //################################################################################################
  virtual glm::vec4 fromLinear(const glm::vec4& color) const=0;

  //################################################################################################
  virtual glm::vec3 fromLinear(const glm::vec3& color) const=0;

  //################################################################################################
  virtual std::string glsl() const=0;
};

}

#endif
