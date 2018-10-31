#ifndef tp_maps_Font_h
#define tp_maps_Font_h

#include "tp_maps/Globals.h"

#include "glm/glm.hpp"

#include <unordered_set>

namespace tp_maps
{
class PreparedString;
struct Pixel;

//##################################################################################################
//! Details of a single character as produced by the Font.
struct Glyph
{
  int w{0};
  int h{0};
  Pixel* data{nullptr};

  float leftBearing  {0.0f}; //Negative for values to the left of 0
  float rightBearing {0.0f}; //Positive to the right of kerningWidth
  float topBearing   {0.0f}; //Positive above 0
  float bottomBearing{0.0f}; //Positive above 0

  float kerningWidth{0.0f};
};

//##################################################################################################
//! Details of a character in a string.
struct GlyphGeometry
{
  glm::vec2 textureCoords[4];
  glm::vec2 vertices[4];

  float leftBearing{0.0f}; //Negative for values to the left of 0
  float rightBearing{0.0f};
  float topBearing{0.0f};    //Positive above 0
  float bottomBearing{0.0f}; //Positive above 0

  float kerningWidth{0.0f};
};

//##################################################################################################
//! Geometry details of a string of characters.
struct FontGeometry
{
  float leftBearing{0.0f}; //Negative for values to the left of 0
  float rightBearing{0.0f};
  float topBearing{0.0f};

  float totalWidth{0.0f};
  float totalHeight{0.0f};

  std::vector<GlyphGeometry> glyphs;
};

//##################################################################################################
//! The Font class is responsible for generating glyphs for rendering text.
/*!
Subclasses of Font are responsible for generating glyphs for a font, the FontRenderer subclasses
take these glyphs a
*/
class TP_MAPS_SHARED_EXPORT Font
{
  friend class PreparedString;
public:
  //################################################################################################
  Font();

  //################################################################################################
  virtual ~Font();

  //################################################################################################
  //! Called to generate a glyph for a character.
  /*!
  This will be called to get a glyph for a character, if the subclass can generate the glyph it
  should call addGlyph with the data for that glyph. The addGlyph function will make a copy of the
  data passed to it.

  \note If the subclass can't generate the glyph it shoul just not call add glyph.

  \param character The character to render.
  \param addGlyph A functor to call with the generated glyph.
  */
  virtual void prepareGlyph(char16_t character, const std::function<void(const Glyph&)>& addGlyph) const = 0;

  //################################################################################################
  //! Optionally this can be implemented to render a place holder for a missing glyph.
  virtual void missingGlyph(const std::function<void(const Glyph&)>& addGlyph) const;

  //################################################################################################
  virtual float lineHeight() const = 0;
};

}

#endif
