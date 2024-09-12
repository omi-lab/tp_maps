#ifndef tp_maps_PostAOMergeShader_h
#define tp_maps_PostAOMergeShader_h

#include "tp_maps/shaders/PostAOBaseShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PostAOMergeShader: public PostAOBaseShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return mergeAmbientOcclusionShaderSID();}

  //################################################################################################
  PostAOMergeShader(Map* map,
                    tp_maps::ShaderProfile shaderProfile,
                    const PostAOParameters& parameters);

  //################################################################################################
  ~PostAOMergeShader();

  //################################################################################################
  void setSSAOTexture(const GLuint id);

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
