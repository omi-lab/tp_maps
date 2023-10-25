#ifndef tp_maps_PostDoFBlurShader_h
#define tp_maps_PostDoFBlurShader_h

#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace tp_maps
{

//##################################################################################################
//! Part of the DoF shaders.
class TP_MAPS_EXPORT PostDoFBlurShader: public PostDoFBaseShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthOfFieldBlurShaderSID();}

  //################################################################################################
  using PostDoFBaseShader::PostDoFBaseShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
