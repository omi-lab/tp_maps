#ifndef tp_maps_FontRenderer_h
#define tp_maps_FontRenderer_h

#include "tp_maps/Globals.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include <unordered_set>
#include <memory>

namespace tp_image_utils
{
class ColorMap;
}

namespace tp_maps
{
class Map;
class Font;
class PreparedString;
struct FontGeometry;
struct Glyph;

//##################################################################################################
class TP_MAPS_EXPORT FontRenderer
{
  friend class PreparedString;
  TP_NONCOPYABLE(FontRenderer);
  TP_DQ;
public:
  //################################################################################################
  FontRenderer(Map* map, const std::shared_ptr<Font>& font);

  //################################################################################################
  virtual ~FontRenderer();

  //################################################################################################
  Map* map() const;

  //################################################################################################
  std::shared_ptr<Font> font() const;

  //################################################################################################
  //! Force a regeneration of the texture using only the required characters.
  void squeeze();

  //################################################################################################
  //! Called when buffers become invalid.
  /*!
  This is called when the OpenGL context becomes invalid, all OpenGL resources should be ignored.
  */
  virtual void invalidateBuffers();

  //################################################################################################
  //! Returns the textureID, binding if required.
  virtual GLuint textureID();

protected:
  //################################################################################################
  virtual void prepareFontGeometry(const PreparedString& preparedString, FontGeometry& fontGeometry);

  //################################################################################################
  virtual void generate();

  //################################################################################################
  virtual void modifyGlyph(const Glyph& glyph, const std::function<void(const Glyph&)>& addGlyph);

  //################################################################################################
  //! Reimplement this if you want to modify the texture.
  virtual void setTexture(const tp_image_utils::ColorMap& texture);

  //################################################################################################
  const std::vector<PreparedString*>& preparedStrings() const;

  //################################################################################################
  const std::unordered_set<char16_t>& requiredCharacters() const;

  //################################################################################################
  void addPreparedString(PreparedString* preparedString);

  //################################################################################################
  void removePreparedString(PreparedString* preparedString);
};

}

#endif
