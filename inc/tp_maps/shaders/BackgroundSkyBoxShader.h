#ifndef tp_maps_BackgroundSkyBoxShader_h
#define tp_maps_BackgroundSkyBoxShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{
//##################################################################################################
//! A shader to render sky spheres.
class TP_MAPS_EXPORT BackgroundSkyBoxShader: public FullScreenShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return backgroundShaderSID();}

  //################################################################################################
  BackgroundSkyBoxShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~BackgroundSkyBoxShader();

  //################################################################################################
  //! Set the texture that will be drawn, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  void setMatrix(const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  //! A rotation value between 0 and 1.
  void setRotationFactor(float rotationFactor);

protected:
  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
