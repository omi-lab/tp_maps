#ifndef tp_maps_PatternShader_h
#define tp_maps_PatternShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_EXPORT BackgroundPatternShader: public FullScreenShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return backgroundPatternShaderSID();}

  //################################################################################################
  BackgroundPatternShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~BackgroundPatternShader() override;

  //################################################################################################
  void setScreenSizeAndGridSpacing(const glm::vec2& screenSize, float gridSpacing);

protected:
  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
