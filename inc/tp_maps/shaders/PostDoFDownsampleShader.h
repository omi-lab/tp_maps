#ifndef tp_maps_PostDoFDownsampleShader_h
#define tp_maps_PostDoFDownsampleShader_h

#include "tp_maps/shaders/PostDoFBaseShader.h"

namespace tp_maps
{

//##################################################################################################
//! Part of the DoF shaders.
class TP_MAPS_EXPORT PostDoFDownsampleShader: public PostDoFBaseShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return downsampleShaderSID();}

  //################################################################################################
  using PostDoFBaseShader::PostDoFBaseShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
