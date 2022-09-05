#ifndef tp_maps_MergeDofShader_h
#define tp_maps_MergeDofShader_h

#include "tp_maps/shaders/DepthOfFieldBlurShader.h"
#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
class MergeDofShader;
namespace detail
{
class MergeDofShaderPrivate
{
  friend class tp_maps::MergeDofShader;
  MergeDofShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_SHARED_EXPORT MergeDofShader: detail::MergeDofShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  MergeDofShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters);

  //################################################################################################
  ~MergeDofShader();

  //################################################################################################
  void compile(const char* vertexShader,
                 const char* fragmentShader,
                 const std::function<void(GLuint)>& bindLocations,
                 const std::function<void(GLuint)>& getLocations,
                 ShaderType shaderType = ShaderType::RenderExtendedFBO);


  //################################################################################################
  void setDownsampledTexture(const GLuint id);
  //################################################################################################
  void setFocusTexture(const GLuint id);
  //################################################################################################
  void setDownsampledFocusTexture( const GLuint id );

  //################################################################################################
  static inline const tp_utils::StringID& name(){return mergeDofShaderSID();}

private:
  using detail::MergeDofShaderPrivate::d;
};

}

#endif
