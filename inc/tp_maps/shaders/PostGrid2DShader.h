#ifndef tp_maps_PostGrid2DShader_h
#define tp_maps_PostGrid2DShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
//! Draw a grid as 2D overlay.
class TP_MAPS_EXPORT PostGrid2DShader: public PostShader
{
  friend class Map;
public:
  //################################################################################################
  PostGrid2DShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return postGrid2DShaderSID();}
};

}

#endif
