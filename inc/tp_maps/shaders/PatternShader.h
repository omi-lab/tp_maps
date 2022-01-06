#ifndef tp_maps_PatternShader_h
#define tp_maps_PatternShader_h

#include "tp_maps/shaders/FullScreenShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_SHARED_EXPORT PatternShader: public FullScreenShader
{
  friend class Map;
public:
  //################################################################################################
  PatternShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~PatternShader();

  //################################################################################################
  void setScreenSizeAndGridSpacing(const glm::vec2& screenSize, float gridSpacing);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return patternShaderSID();}

private:
  struct Private;
  Private* d;
};

}

#endif
