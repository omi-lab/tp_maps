#ifndef tp_maps_PostBasicBlurShader_h
#define tp_maps_PostBasicBlurShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Add a blur in post processing.
class TP_MAPS_EXPORT PostBasicBlurShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postBasicBlurShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
