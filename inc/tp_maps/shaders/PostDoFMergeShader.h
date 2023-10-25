#ifndef tp_maps_PostDoFMergeShader_h
#define tp_maps_PostDoFMergeShader_h

#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace tp_maps
{

//##################################################################################################
//! Part of the DoF shaders.
class TP_MAPS_EXPORT PostDoFMergeShader: public PostDoFBaseShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return mergeDofShaderSID();}

  //################################################################################################
  PostDoFMergeShader(Map* map,
                     tp_maps::OpenGLProfile openGLProfile,
                     const PostDoFParameters& parameters);

  //################################################################################################
  ~PostDoFMergeShader();

  //################################################################################################
  void setDownsampledTexture(GLuint id);

  //################################################################################################
  void setFocusTexture(GLuint id);

  //################################################################################################
  void setDownsampledFocusTexture(GLuint id);

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
