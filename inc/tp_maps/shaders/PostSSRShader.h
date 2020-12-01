#ifndef tp_maps_PostSSRShader_h
#define tp_maps_PostSSRShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for Screen Space Reflection.
class TP_MAPS_SHARED_EXPORT PostSSRShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostSSRShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postSSRShaderSID();}
};

}

#endif
