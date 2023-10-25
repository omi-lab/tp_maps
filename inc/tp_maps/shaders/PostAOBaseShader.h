#ifndef tp_maps_PostAOBaseShader_h
#define tp_maps_PostAOBaseShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{
//##################################################################################################
struct PostAOParameters
{
  bool enabled{false};
  bool showTexture{false};
  bool useScreenBuffer{true};
  bool useLightBuffers{false};
  float radius{0.05f};
  size_t nSamples{64};
  float bias{0.000025f};
  float boostUpperThreshold{0.8f};
  float boostLowerThreshold{0.4f};
  float boostUpperFactor{0.9f};
  float boostLowerFactor{0.8f};
};

//##################################################################################################
//! A shader for Screen Space Ambient Occlusion.
class TP_MAPS_EXPORT PostAOBaseShader: public PostShader
{
  TP_DQ;
public:
  //################################################################################################
  PostAOBaseShader(Map* map,
                   tp_maps::OpenGLProfile openGLProfile,
                   const PostAOParameters& parameters);

  //################################################################################################
  ~PostAOBaseShader();

  //################################################################################################
  const PostAOParameters& parameters() const;
};

}

#endif
