#ifndef tp_maps_PostOutlineShader_h
#define tp_maps_PostOutlineShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PostOutlineShader: public PostShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return postOutlineShaderSID();}

  //################################################################################################
  PostOutlineShader(tp_maps::Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~PostOutlineShader();

  //################################################################################################
  void use(tp_maps::ShaderType shaderType) override;

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, tp_maps::ShaderType shaderType) override;
};

}

#endif
