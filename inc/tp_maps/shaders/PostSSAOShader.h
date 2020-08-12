#ifndef tp_maps_PostSSAOShader_h
#define tp_maps_PostSSAOShader_h

#include "tp_maps/Shader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_SHARED_EXPORT PostSSAOShader: public Shader
{
  friend class Map;
public:
  //################################################################################################
  PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader=nullptr, const char* fragmentShader=nullptr);

  //################################################################################################
  ~PostSSAOShader() override;

  //################################################################################################
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  void setReflectionTextures(GLuint colorID, GLuint depthID);

  //################################################################################################
  void setProjectionMatrix(const glm::mat4& projectionMatrix);

  //################################################################################################
  void draw();

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSAOShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
