#ifndef tp_maps_DownsampleShader_h
#define tp_maps_DownsampleShader_h

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"
#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
class DownsampleShader;
namespace detail
{
class DownsampleShaderPrivate
{
  friend class tp_maps::DownsampleShader;
  DownsampleShaderPrivate(tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_SHARED_EXPORT DownsampleShader: detail::DownsampleShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  DownsampleShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);

  //################################################################################################
  ~DownsampleShader();

  //################################################################################################
  void compile(const char* vertexShader,
                 const char* fragmentShader,
                 const std::function<void(GLuint)>& bindLocations,
                 const std::function<void(GLuint)>& getLocations,
                 ShaderType shaderType = ShaderType::RenderExtendedFBO);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return downsampleShaderSID();}

private:
  using detail::DownsampleShaderPrivate::d;
};

}

#endif
