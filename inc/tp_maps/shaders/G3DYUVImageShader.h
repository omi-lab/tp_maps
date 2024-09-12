#ifndef tp_maps_YUVImageShader_h
#define tp_maps_YUVImageShader_h

#include "tp_maps/shaders/G3DImageShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering YUV image data.
class TP_MAPS_EXPORT G3DYUVImageShader: public G3DImageShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return yuvImageShaderSID();}

  //################################################################################################
  using G3DImageShader::G3DImageShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
