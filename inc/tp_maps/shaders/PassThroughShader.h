#ifndef tp_maps_PassThroughShader_h
#define tp_maps_PassThroughShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PassThroughShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return passThroughShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
