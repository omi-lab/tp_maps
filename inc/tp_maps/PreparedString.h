#ifndef tp_maps_PreparedString_h
#define tp_maps_PreparedString_h

#include "tp_maps/Globals.h"

#include <string>

namespace tp_maps
{
class FontRenderer;
struct FontGeometry;

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT PreparedStringConfig
{
  bool topDown{false};

  //! Offset the text relative to the size of the block of text.
  glm::vec2 relativeOffset{0.0f, 0.0f};

  //! Offset the text in pixels.
  glm::vec2 pixelOffset{0.0f, 0.0f};
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT PreparedString
{
public:
  TP_NONCOPYABLE(PreparedString);

  //################################################################################################
  //! Construct a new string renderable.
  /*!
  \warning The font must be valid for the life of this object.
  \param font The font to use for rendering the text.
  \param text The text to render.
  */
  PreparedString(FontRenderer* fontRenderer, const std::u16string& text, const PreparedStringConfig& config=PreparedStringConfig());

  //################################################################################################
  virtual ~PreparedString();

  //################################################################################################
  const std::u16string& text() const;

  //################################################################################################
  //! Returns the textureID, binding if required.
  GLuint textureID() const;

  //################################################################################################
  const FontGeometry& fontGeometry() const;

  //################################################################################################
  const PreparedStringConfig& config() const;

protected:

  //################################################################################################
  //! Called when buffers become invalid.
  /*!
  This is called when the OpenGL context becomes invalid, all OpenGL resources should be ignored.
  */
  virtual void invalidateBuffers();

  //################################################################################################
  //! Called to force the buffers to be recreated.
  /*!
  The OpenGL buffers are still valid but the data that they were generated from has changed. As a
  result the font geometry should be recreated next time fontGeometry is called.
  */
  virtual void regenerateBuffers();

private:
  struct Private;
  Private* d;
  friend struct Private;
  friend class FontRenderer;
};

}

#endif
