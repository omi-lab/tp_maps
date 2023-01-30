#ifndef tp_maps_DepthShader_h
#define tp_maps_DepthShader_h

#include "tp_maps/shaders/ImageShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering mesh depth values to the output buffer.
/*!
This is for rendering the depth of a mesh to the color buffer.
*/
class TP_MAPS_EXPORT DepthShader: public ImageShader
{
public:
  //################################################################################################
  DepthShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthShaderSID();}
};

}

#endif
