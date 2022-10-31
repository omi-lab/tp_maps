#ifndef tp_maps_MergeAmbientOcclusionShader_h
#define tp_maps_MergeAmbientOcclusionShader_h

#include "tp_maps/shaders/PostShader.h"

#include "tp_maps/shaders/AmbientOcclusionParameters.h"

namespace tp_maps
{

class MergeAmbientOcclusionShader;
namespace detail
{
class MergeAmbientOcclusionShaderPrivate
{
  friend class tp_maps::MergeAmbientOcclusionShader;
  MergeAmbientOcclusionShaderPrivate(tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_SHARED_EXPORT MergeAmbientOcclusionShader: detail::MergeAmbientOcclusionShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  MergeAmbientOcclusionShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return mergeAmbientOcclusionShaderSID();}

  //################################################################################################
  void setSSAOTexture(const GLuint id);

private:
  using detail::MergeAmbientOcclusionShaderPrivate::d;
};

}

#endif
