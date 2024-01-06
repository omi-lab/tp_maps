#ifndef tp_maps_Buffers_h
#define tp_maps_Buffers_h

#include "tp_maps/Globals.h"

namespace tp_maps
{
class Map;

//##################################################################################################
class TP_MAPS_EXPORT Buffers
{
  TP_NONCOPYABLE(Buffers);
public:
  //################################################################################################
  Buffers(Map* map);

  //################################################################################################
  ~Buffers();

  //################################################################################################
  bool prepareBuffer(const std::string& name,
                     FBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     ExtendedFBO extendedFBO,
                     bool clear) const;

  //################################################################################################
  void invalidateBuffer(FBO& fbo) const;

  //################################################################################################
  void deleteBuffer(FBO& fbo) const;

  //################################################################################################
  void swapMultisampledBuffer(FBO& fbo) const;

  //################################################################################################
  void setDrawBuffers(const std::vector<GLenum>& buffers) const;

  //################################################################################################
  void setMaxSamples(size_t maxSamples);

  //################################################################################################
  size_t maxSamples() const;

  //################################################################################################
  void initializeGL();

  //################################################################################################
  std::unordered_map< std::string, FBO* > storedBuffers() const;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
