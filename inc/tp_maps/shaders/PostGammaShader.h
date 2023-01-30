#ifndef tp_maps_PostGammaShader_h
#define tp_maps_PostGammaShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Applies gamma correction
class TP_MAPS_EXPORT PostGammaShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostGammaShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postGammaShaderSID();}
};

}

#endif
