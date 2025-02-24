#ifndef tp_maps_PostTwoPassBlurShader_h
#define tp_maps_PostTwoPassBlurShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

// ##################################################################################################
//! Add a blur in post processing.
class TP_MAPS_EXPORT PostTwoPassBlurShader : public PostShader
{
  TP_DQ;
public:
  // ################################################################################################
  static inline const tp_utils::StringID &name() { return postTwoPassBlurShaderSID(); }

  //################################################################################################
  PostTwoPassBlurShader(tp_maps::Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~PostTwoPassBlurShader();

  //################################################################################################
  void use(tp_maps::ShaderType shaderType) override;

  //################################################################################################
  // This is used in the 'use' method and should be set before that is called
  int   dirIndex   {0};  // 0: horizontal, 1: vertical
  float blurRadius {4};
  float blurSigma  {4};

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, tp_maps::ShaderType shaderType) override;
};

}

#endif // tp_maps_PostTwoPassBlurShader_h
