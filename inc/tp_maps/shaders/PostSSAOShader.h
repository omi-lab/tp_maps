#ifndef tp_maps_PostSSAOShader_h
#define tp_maps_PostSSAOShader_h

#include "tp_maps/shaders/PostShader.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

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
  float bias{0.00025f};
};

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_EXPORT PostSSAOShader: public PostShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSAOShaderSID();}

  //################################################################################################
  PostSSAOShader(Map* map,
                 tp_maps::ShaderProfile shaderProfile,
                 const PostSSAOParameters& parameters);

  //################################################################################################
  ~PostSSAOShader();

  //################################################################################################
  void setLights(const std::vector<tp_math_utils::Light>& lights, const std::vector<OpenGLFBO>& lightBuffers);

  //################################################################################################
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
