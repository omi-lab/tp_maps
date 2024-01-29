#ifndef tp_maps_PostShader_h
#define tp_maps_PostShader_h

#include "tp_maps/shaders/FullScreenShader.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
//! The base class for post processing shaders.
class TP_MAPS_EXPORT PostShader: public FullScreenShader
{
public:
  //################################################################################################
    PostShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~PostShader();

  //################################################################################################
  void setReadFBO(const OpenGLFBO& readFBO);

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
