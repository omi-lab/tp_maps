#ifndef tp_maps_PostSSRShader_h
#define tp_maps_PostSSRShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Reflection.
class TP_MAPS_EXPORT PostSSRShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSRShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
