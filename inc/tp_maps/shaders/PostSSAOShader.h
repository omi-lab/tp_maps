#ifndef tp_maps_PostSSAOShader_h
#define tp_maps_PostSSAOShader_h

#include "tp_maps/shaders/PostShader.h"

#include "tp_math_utils/Light.h"

namespace tp_maps
{

//##################################################################################################
struct PostSSAOParameters
{
  bool useScreenBuffer{true};
  bool useLightBuffers{false};
  float radius{0.05f};
  size_t nSamples{64};
  float bias{0.00025};
};

//##################################################################################################
class PostSSAOShader;
namespace detail
{
class PostSSAOShaderPrivate
{
  friend class tp_maps::PostSSAOShader;
  PostSSAOShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const PostSSAOParameters& parameters);
  struct Private;
  Private* d;
};
}

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_SHARED_EXPORT PostSSAOShader: detail::PostSSAOShaderPrivate, public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const PostSSAOParameters& parameters);

  //################################################################################################
  ~PostSSAOShader();

  //################################################################################################
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSAOShaderSID();}

private:

  //################################################################################################
  //! Called by use()
  void setLights(const std::vector<tp_math_utils::Light>& lights, const std::vector<FBO>& lightBuffers);

  using detail::PostSSAOShaderPrivate::d;
};

}

#endif
