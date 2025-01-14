#ifndef tp_maps_OpenGLBuffers_h
#define tp_maps_OpenGLBuffers_h

#include "tp_maps/subsystems/Subsystem.h"
#ifdef TP_MAPS_SUBSYSTEM_OPENGL

#include "tp_maps/subsystems/open_gl/OpenGL.h"

namespace tp_maps
{
class Map;

//##################################################################################################
class TP_MAPS_EXPORT OpenGLBuffers
{
  TP_NONCOPYABLE(OpenGLBuffers);
  TP_DQ;
public:
  //################################################################################################
  OpenGLBuffers(Map* map);

  //################################################################################################
  ~OpenGLBuffers();

  //################################################################################################
  bool prepareBuffer(OpenGLFBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     ExtendedFBO extendedFBO,
                     bool clear) const;

  //################################################################################################
  bool bindBuffer(OpenGLFBO& buffer) const;

  //################################################################################################
  void invalidateBuffer(OpenGLFBO& fbo) const;

  //################################################################################################
  void deleteBuffer(OpenGLFBO& fbo) const;

  //################################################################################################
  void swapMultisampledBuffer(OpenGLFBO& fbo) const;

  //################################################################################################
  void setDrawBuffers(const std::vector<GLenum>& buffers) const;

  //################################################################################################
  void setMaxSamples(size_t maxSamples);

  //################################################################################################
  size_t maxSamples() const;

  //################################################################################################
  void initializeGL();

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, OpenGLFBO*>& storedBuffers() const;
};

}

#endif
#endif
