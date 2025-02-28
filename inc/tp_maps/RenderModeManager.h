#ifndef tp_maps_RenderModeManager_h
#define tp_maps_RenderModeManager_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;

//##################################################################################################
class TP_MAPS_EXPORT RenderModeManager
{
  TP_NONCOPYABLE(RenderModeManager);
  TP_DQ;
public:
  //################################################################################################
  RenderModeManager(Map* map);

  //################################################################################################
  ~RenderModeManager();

  //################################################################################################
  void setDefaultRenderMode(RenderMode defaultRenderMode);

  //################################################################################################
  void setRenderMode(RenderMode renderMode);

  //################################################################################################
  RenderMode renderMode() const;

  //################################################################################################
  void setShadowSamples(RenderMode renderMode, size_t shadowSamples);

  //################################################################################################
  //! The number of adjacent samples to take from a shadow texture
  /*!
  The actual number of samples that is made is as follows:

  samples = (shadowSamples+1) * (shadowSamples+1) * nLights;
  */
  size_t shadowSamples(RenderMode renderMode) const;

  //################################################################################################
  size_t shadowSamples() const;

  //################################################################################################
  bool isDoFRendered() const;

  //##################################################################################################
  bool msaaAllowed() const;

  //################################################################################################
  void enterFastRenderMode();
};

}

#endif
