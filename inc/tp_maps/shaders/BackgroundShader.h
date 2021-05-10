#ifndef tp_maps_BackgroundShader_h
#define tp_maps_BackgroundShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_SHARED_EXPORT BackgroundShader: public FullScreenShader
{
  friend class Map;
public:
  //################################################################################################
  BackgroundShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~BackgroundShader();

  //################################################################################################
  //! Set the texture that will be drawn, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  void setMatrix(const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  //! A rotation value between 0 and 1.
  void setRotationFactor(float rotationFactor);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return backgroundShaderSID();}

private:
  struct Private;
  Private* d;
};

}

#endif
