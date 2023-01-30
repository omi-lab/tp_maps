#ifndef tp_maps_AmbientOcclusionShader_h
#define tp_maps_AmbientOcclusionShader_h

#include "tp_maps/shaders/PostShader.h"

#include "tp_maps/shaders/AmbientOcclusionParameters.h"

namespace tp_maps
{

//##################################################################################################
class AmbientOcclusionShader;
namespace detail
{
class AmbientOcclusionShaderPrivate
{
  friend class tp_maps::AmbientOcclusionShader;
  AmbientOcclusionShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_EXPORT AmbientOcclusionShader: detail::AmbientOcclusionShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  AmbientOcclusionShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters);

  //################################################################################################
  ~AmbientOcclusionShader();

  //################################################################################################
  void compile(const char* vertexShader,
                 const char* fragmentShader,
                 const std::function<void(GLuint)>& bindLocations,
                 const std::function<void(GLuint)>& getLocations,
                 ShaderType shaderType = ShaderType::RenderExtendedFBO) override;

  //################################################################################################
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  static inline const tp_utils::StringID& name(){return ambientOcclusionShaderSID();}

private:
  using detail::AmbientOcclusionShaderPrivate::d;
};

}

#endif
