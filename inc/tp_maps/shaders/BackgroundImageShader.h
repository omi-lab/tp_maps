#ifndef tp_maps_BackgroundImageShader_h
#define tp_maps_BackgroundImageShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader to render images in the background.
class TP_MAPS_EXPORT BackgroundImageShader: public FullScreenShader
{
  friend class Map;
public:
  //################################################################################################
  BackgroundImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~BackgroundImageShader();

  //################################################################################################
  //! Set the texture that will be drawn, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return backgroundImageShaderSID();}

private:
  struct Private;
  Private* d;
};

}

#endif
