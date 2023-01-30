#ifndef tp_maps_PassThroughShader_h
#define tp_maps_PassThroughShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw outlines around a mask rendered to the previous FBO.
class TP_MAPS_EXPORT PassThroughShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PassThroughShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return passThroughShaderSID();}
};

}

#endif
