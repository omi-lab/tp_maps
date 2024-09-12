#ifndef tp_maps_PostGammaShader_h
#define tp_maps_PostGammaShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Applies gamma correction
class TP_MAPS_EXPORT PostGammaShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postGammaShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
