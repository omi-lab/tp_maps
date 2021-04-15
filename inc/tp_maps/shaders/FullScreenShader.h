#ifndef tp_maps_FullScreenShader_h
#define tp_maps_FullScreenShader_h

#include "tp_maps/Shader.h"

namespace tp_maps
{

//##################################################################################################
//! The base class for post processing shaders.
class TP_MAPS_SHARED_EXPORT FullScreenShader: public Shader
{
public:
  //################################################################################################
  FullScreenShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~FullScreenShader() override;

  //################################################################################################
  void compile(const char* vertexShader,
               const char* fragmentShader,
               const std::function<void(GLuint)>& bindLocations,
               const std::function<void(GLuint)>& getLocations,
               ShaderType shaderType = ShaderType::Render);

  //################################################################################################
  void draw();

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
