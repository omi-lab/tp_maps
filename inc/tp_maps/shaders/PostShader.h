#ifndef tp_maps_PostShader_h
#define tp_maps_PostShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! The base class for post processing shaders.
class TP_MAPS_EXPORT PostShader: public FullScreenShader
{
public:
  //################################################################################################
  PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~PostShader();

  //################################################################################################
  void setReadFBO(const FBO& readFBO);

  //################################################################################################
  void setFBOSourceTexture(const GLuint id);

  //################################################################################################
  void setProjectionMatrix(const glm::mat4& projectionMatrix);

protected:
  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void init() override;

private:
  struct Private;
  Private* d;
};

}

#endif
