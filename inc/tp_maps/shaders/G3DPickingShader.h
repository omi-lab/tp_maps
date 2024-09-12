#ifndef tp_maps_G3DPickingShader_h
#define tp_maps_G3DPickingShader_h

#include "tp_maps/shaders/G3DImageShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw the picking results and make it colorful.
class TP_MAPS_EXPORT G3DPickingShader: public G3DImageShader
{
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return pickingShaderSID();}

  //################################################################################################
  using G3DImageShader::G3DImageShader;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;
};

}

#endif
