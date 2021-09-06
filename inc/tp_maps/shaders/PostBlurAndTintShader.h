#ifndef tp_maps_PostBlurAndTintShader_h
#define tp_maps_PostBlurAndTintShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_SHARED_EXPORT PostBlurAndTintShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostBlurAndTintShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postBlurAndTintShaderSID();}
};

}

#endif
