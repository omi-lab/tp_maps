#ifndef tp_maps_PostBlurAndTintShader_h
#define tp_maps_PostBlurAndTintShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PostBlurAndTintShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postBlurAndTintShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
