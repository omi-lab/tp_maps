#ifndef tp_maps_PostShader_h
#define tp_maps_PostShader_h

#include "tp_maps/Shader.h"

namespace tp_maps
{

//##################################################################################################
//! The base class for post processing shaders.
class TP_MAPS_SHARED_EXPORT PostShader: public Shader
{
  friend class Map;
public:
  //################################################################################################
  PostShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader);

  //################################################################################################
  ~PostShader() override;

  //################################################################################################
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  void setReflectionTextures(GLuint colorID, GLuint depthID);

  //################################################################################################
  void setProjectionMatrix(const glm::mat4& projectionMatrix);

  //################################################################################################
  void draw();

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
