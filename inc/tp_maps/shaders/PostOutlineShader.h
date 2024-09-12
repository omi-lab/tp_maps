#ifndef tp_maps_PostOutlineShader_h
#define tp_maps_PostOutlineShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PostOutlineShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postOutlineShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
