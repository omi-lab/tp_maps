#ifndef tp_maps_DepthImageShader_h
#define tp_maps_DepthImageShader_h

#include "tp_maps/shaders/ImageShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering depth buffers.
class TP_MAPS_SHARED_EXPORT DepthImageShader: public ImageShader
{
public:
  //################################################################################################
  DepthImageShader(tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthImageShaderSID();}
};

}

#endif
