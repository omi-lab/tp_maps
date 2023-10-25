#ifndef tp_maps_PostBlitShader_h
#define tp_maps_PostBlitShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader that blits the input buffer to the output.
class TP_MAPS_EXPORT PostBlitShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postBlitShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
