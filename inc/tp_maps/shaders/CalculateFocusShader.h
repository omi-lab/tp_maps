#ifndef tp_maps_CalculateFocusShader_h
#define tp_maps_CalculateFocusShader_h

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"
#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
class CalculateFocusShader;
namespace detail
{
class CalculateFocusShaderPrivate
{
  friend class tp_maps::CalculateFocusShader;
  CalculateFocusShaderPrivate(tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT CalculateFocusShader: detail::CalculateFocusShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  CalculateFocusShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);

  //################################################################################################
  ~CalculateFocusShader();

  //################################################################################################
  void compile(const char* vertexShader,
                 const char* fragmentShader,
                 const std::function<void(GLuint)>& bindLocations,
                 const std::function<void(GLuint)>& getLocations,
                 ShaderType shaderType = ShaderType::RenderExtendedFBO);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return calculateFocusShaderSID();}

private:
  using detail::CalculateFocusShaderPrivate::d;
};

}

#endif
