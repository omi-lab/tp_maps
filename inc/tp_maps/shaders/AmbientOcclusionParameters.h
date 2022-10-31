#ifndef tp_maps_AmbientOcclusionParameters_h
#define tp_maps_AmbientOcclusionParameters_h

#include <cstddef>

// Good values
// float radius{0.05f};
// float bias{0.00025f};

namespace tp_maps
{
//##################################################################################################
struct AmbientOcclusionParameters
{
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

}

#endif // tp_maps_AmbientOcclusionParameters_h
