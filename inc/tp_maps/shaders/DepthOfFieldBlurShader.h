#ifndef tp_maps_DepthOfFieldBlurShader_h
#define tp_maps_DepthOfFieldBlurShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

////##################################################################################################
struct DepthOfFieldShaderParameters
{
  float depthOfField{2.0f}; // focal distance
  float fStop{5.0f};

  // Calculate focus parameters
  float nearPlane{0.1f};
  float farPlane{50.0f};
  float focalDistance{8.0f};
  float blurinessCutoffConstant{10.0f};
};

//##################################################################################################
class DepthOfFieldBlurShader;
namespace detail
{
class DepthOfFieldBlurShaderPrivate
{
  friend class tp_maps::DepthOfFieldBlurShader;
  DepthOfFieldBlurShaderPrivate(tp_maps::OpenGLProfile openGLProfile,
                                const DepthOfFieldShaderParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_SHARED_EXPORT DepthOfFieldBlurShader:
    detail::DepthOfFieldBlurShaderPrivate,
    public PostShader
{
  friend class Map;
public:
  //################################################################################################
  DepthOfFieldBlurShader(Map* map,
                         tp_maps::OpenGLProfile openGLProfile,
                         const DepthOfFieldShaderParameters& parameters);

  //################################################################################################
  ~DepthOfFieldBlurShader();

  //################################################################################################
  void compile(const char* vertexShader,
                 const char* fragmentShader,
                 const std::function<void(GLuint)>& bindLocations,
                 const std::function<void(GLuint)>& getLocations,
                 ShaderType shaderType = ShaderType::RenderExtendedFBO);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthOfFieldBlurShaderSID();}

private:
  using detail::DepthOfFieldBlurShaderPrivate::d;
};

}

#endif
