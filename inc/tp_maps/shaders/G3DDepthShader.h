#ifndef tp_maps_DepthShader_h
#define tp_maps_DepthShader_h

#include "tp_maps/shaders/G3DImageShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering mesh depth values to the output buffer.
/*!
This is for rendering the depth of a mesh to the color buffer.
*/
class TP_MAPS_EXPORT G3DDepthShader: public G3DImageShader
{
public:  
  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthShaderSID();}

  //################################################################################################
  using G3DImageShader::G3DImageShader;

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
