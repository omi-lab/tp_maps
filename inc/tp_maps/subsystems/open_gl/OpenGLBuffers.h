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
public:
  //################################################################################################
  OpenGLBuffers(Map* map);

  //################################################################################################
  ~OpenGLBuffers();

  //################################################################################################
  bool prepareBuffer(const std::string& name,
                     OpenGLFBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     ExtendedFBO extendedFBO,
                     bool clear) const;

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
  std::unordered_map<std::string, OpenGLFBO*> storedBuffers() const;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
#endif
