#ifndef tp_maps_Font_h
#define tp_maps_Font_h

#include "tp_maps/Globals.h"

#include "glm/glm.hpp"

#include <unordered_set>
#include <array>

union TPPixel;

namespace tp_maps
{
class PreparedString;

//##################################################################################################
//! Details of a single character as produced by the Font.
struct TP_MAPS_SHARED_EXPORT Glyph
{
  int w{0};
  int h{0};
  TPPixel* data{nullptr};

  // Coordinate sysatem
  // 1
  // ^
  // |
  // |
  // 0----> 1

  float leftBearing  {0.0f}; //Negative for values to the left of 0
  float rightBearing {0.0f}; //Positive to the right of kerningWidth
  float topBearing   {0.0f}; //Positive above 0
  float bottomBearing{0.0f}; //Positive above 0

  float kerningWidth{0.0f};
};

//##################################################################################################
//! Details of a character in a string.
struct TP_MAPS_SHARED_EXPORT GlyphGeometry
{
  // Coordinate sysatem
  // 1
  // ^
  // |
  // |
  // 0----> 1

  std::array<glm::vec2, 4> textureCoords; // (Bottom left, Bottom right, Top right, Top left)
  std::array<glm::vec2, 4> vertices;      // (Bottom left, Bottom right, Top right, Top left)

  float leftBearing{0.0f}; //Negative for values to the left of 0
  float rightBearing{0.0f};
  float topBearing{0.0f};    //Positive above 0
  float bottomBearing{0.0f}; //Positive above 0

  float kerningWidth{0.0f};
};

//##################################################################################################
//! Geometry details of a string of characters.
struct TP_MAPS_SHARED_EXPORT FontGeometry
{
  // Coordinate sysatem
  // 1
  // ^
  // |
  // |
  // 0----> 1

  float leftBearing  {0.0f}; //Negative for values to the left of 0
  float rightBearing {0.0f}; //Negative for values to the left of 0
  float topBearing   {0.0f}; //Positive above 0

  float totalWidth  {0.0f};
  float totalHeight {0.0f};

  float top    {0.0f};
  float bottom {0.0f};
  float left   {0.0f};
  float right  {0.0f};

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
