#ifndef tp_maps_DepthImageShader_h
#define tp_maps_DepthImageShader_h

#include "tp_maps/shaders/ImageShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering depth buffers.
/*!
This is for rendering a depth texture to the screen to view it. If you want to render the depth
values of a mesh use DepthShader instead.

This linearizes the depth values to make them easier to visualize. This shader is kind of broke as
it does not pass the near/far planes in.
*/
class TP_MAPS_SHARED_EXPORT DepthImageShader: public ImageShader
{
public:
  //################################################################################################
  DepthImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthImageShaderSID();}
};

//##################################################################################################
//! A shader for rendering depth buffers.
class TP_MAPS_SHARED_EXPORT DepthImage3DShader: public ImageShader
{
public:
  //################################################################################################
  DepthImage3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return depthImage3DShaderSID();}
};

}

#endif
