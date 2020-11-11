#ifndef tp_maps_PostSSAOShader_h
#define tp_maps_PostSSAOShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_SHARED_EXPORT PostSSAOShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostSSAOShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSAOShaderSID();}
};

}

#endif
