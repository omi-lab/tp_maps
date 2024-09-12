#ifndef tp_maps_PostGrid2DShader_h
#define tp_maps_PostGrid2DShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw a grid as 2D overlay.
class TP_MAPS_EXPORT PostGrid2DShader: public PostShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postGrid2DShaderSID();}

  //################################################################################################
  using PostShader::PostShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
