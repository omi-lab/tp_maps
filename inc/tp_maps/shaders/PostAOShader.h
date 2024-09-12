#ifndef tp_maps_PostAOShader_h
#define tp_maps_PostAOShader_h

#include "tp_maps/shaders/PostAOBaseShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_EXPORT PostAOShader: public PostAOBaseShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return ambientOcclusionShaderSID();}

  //################################################################################################
  PostAOShader(Map* map,
               tp_maps::ShaderProfile shaderProfile,
               const PostAOParameters& parameters);

  //################################################################################################
  ~PostAOShader();

  //################################################################################################
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void invalidate() override;
};

}

#endif
