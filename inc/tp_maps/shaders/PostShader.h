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
  PostShader(Map* map,
             tp_maps::OpenGLProfile openGLProfile,
             const char* vertexShader,
             const char* fragmentShader,
             const std::function<void(GLuint)>& bindLocations=std::function<void(GLuint)>(),
             const std::function<void(GLuint)>& getLocations=std::function<void(GLuint)>());

  //################################################################################################
  ~PostShader();

  //################################################################################################
  virtual void compile(const char* vertexShader,
               const char* fragmentShader,
               const std::function<void(GLuint)>& bindLocations,
               const std::function<void(GLuint)>& getLocations,
               ShaderType shaderType = ShaderType::RenderExtendedFBO);

  //################################################################################################
  void setReadFBO(const FBO& readFBO);

  //################################################################################################
  void setFBOSourceTexture(const GLuint id);

  //################################################################################################
  void setProjectionMatrix(const glm::mat4& projectionMatrix);

private:
  struct Private;
  Private* d;
};

}

#endif
