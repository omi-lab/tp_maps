#ifndef tp_maps_YUVImageShader_h
#define tp_maps_YUVImageShader_h

#include "tp_maps/shaders/ImageShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering YUV image data.
class TP_MAPS_EXPORT YUVImageShader: public ImageShader
{
public:
  //################################################################################################
  YUVImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return yuvImageShaderSID();}
};

}

#endif
