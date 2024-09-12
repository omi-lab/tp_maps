#ifndef tp_maps_G3DDepthImageShader_h
#define tp_maps_G3DDepthImageShader_h

#include "tp_maps/shaders/G3DImageShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering depth buffers.
/*!
This is for rendering a depth texture to the screen to view it. If you want to render the depth
values of a mesh use DepthShader instead.

This linearizes the depth values to make them easier to visualize. This shader is kind of broke as
it does not pass the near/far planes in.
*/
class TP_MAPS_EXPORT G3DDepthImageShader: public G3DImageShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthImageShaderSID();}

  //################################################################################################
  using G3DImageShader::G3DImageShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
