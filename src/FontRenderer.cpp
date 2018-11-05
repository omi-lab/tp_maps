#include "tp_maps/FontRenderer.h"
#include "tp_maps/Font.h"
#include "tp_maps/PreparedString.h"
#include "tp_maps/Map.h"
#include "tp_maps/textures/BasicTexture.h"

#include "tp_utils/TimeUtils.h"
#include "tp_utils/DebugUtils.h"

#include <unordered_map>

namespace tp_maps
{
//##################################################################################################
struct FontRenderer::Private
{
  FontRenderer* q;
  Map* map;
  Font* font;

  std::vector<PreparedString*> preparedStrings;
  std::unordered_set<char16_t> requiredCharacters;

  BasicTexture texture;

  std::unordered_map<char16_t, GlyphGeometry> glyphGeometry;
  GlyphGeometry missingGeometry;
  GLuint textureID{0};

  bool bindTexture{false};
  bool regenerate{false};

  //################################################################################################
  Private(FontRenderer* q_, Map* map_, Font* font_):
    q(q_),
    map(map_),
    font(font_),
    texture(map)
  {

  }

  //################################################################################################
  void freeAndInvalidate()
  {
    freeTexture();
    regenerate = true;
  }

  //################################################################################################
  void freeTexture()
  {
    if(textureID)
    {
      map->makeCurrent();
      map->deleteTexture(textureID);
      textureID = 0;
      bindTexture = false;
    }
  }

  //################################################################################################
  void generate()
  {
    if(!regenerate)
      return;
    regenerate = false;
    q->generate();
  }
};

//##################################################################################################
FontRenderer::FontRenderer(Map* map, Font* font):
  d(new Private(this, map, font))
{
  d->map->addFontRenderer(this);
}

//##################################################################################################
FontRenderer::~FontRenderer()
{
  d->map->removeFontRenderer(this);
  delete d;
}

//##################################################################################################
Map* FontRenderer::map() const
{
  return d->map;
}

//##################################################################################################
Font* FontRenderer::font() const
{
  return d->font;
}

//##################################################################################################
void FontRenderer::squeeze()
{
  d->requiredCharacters.clear();
  d->freeAndInvalidate();
}

//##################################################################################################
void FontRenderer::invalidateBuffers()
{
  d->textureID = 0;
  for(auto preparedString : preparedStrings())
    preparedString->invalidateBuffers();
}

//##################################################################################################
GLuint FontRenderer::textureID()
{
  d->generate();

  if(d->bindTexture)
  {
    d->bindTexture = false;
    d->map->deleteTexture(d->textureID);
    d->textureID = d->texture.bindTexture();
  }

  return d->textureID;
}

//##################################################################################################
void FontRenderer::prepareFontGeometry(const PreparedString& preparedString, FontGeometry& fontGeometry)
{
  tp_utils::ElapsedTimer t;
  t.start();

  d->generate();

  float lineSpacing=d->font->lineHeight();

  fontGeometry.leftBearing  = 0.0f;
  fontGeometry.rightBearing = 0.0f;
  fontGeometry.topBearing   = 0.0f;

  fontGeometry.totalWidth   = 0.0f;
  fontGeometry.totalHeight  = lineSpacing;

  const auto& text = preparedString.text();

  fontGeometry.glyphs.resize(text.size());
  size_t count=0;
  size_t row=0;
  glm::vec2 offset{0.0f, 0.0f};
  for(const auto character : text)
  {
    if(character == '\n')
    {
      offset.x = 0.0f;
      offset.y += lineSpacing;
      fontGeometry.totalHeight += lineSpacing;
      row++;
    }

    const auto& geometry = [&]
    {
      const auto i = d->glyphGeometry.find(character);
      return (i==d->glyphGeometry.end())?d->missingGeometry:i->second;
    }();

    if(count==0 || geometry.leftBearing<fontGeometry.leftBearing)
      fontGeometry.leftBearing = geometry.leftBearing;

    if(row==0 && geometry.topBearing>fontGeometry.topBearing)
      fontGeometry.topBearing = geometry.topBearing;

    auto& outGeometry = fontGeometry.glyphs[count];

    outGeometry.textureCoords[0] = geometry.textureCoords[0];
    outGeometry.textureCoords[1] = geometry.textureCoords[1];
    outGeometry.textureCoords[2] = geometry.textureCoords[2];
    outGeometry.textureCoords[3] = geometry.textureCoords[3];

    outGeometry.vertices[0] = geometry.vertices[0] + offset;
    outGeometry.vertices[1] = geometry.vertices[1] + offset;
    outGeometry.vertices[2] = geometry.vertices[2] + offset;
    outGeometry.vertices[3] = geometry.vertices[3] + offset;

    count++;

    offset.x += geometry.kerningWidth;

    if(offset.x > fontGeometry.totalWidth)
      fontGeometry.totalWidth = offset.x;
  }

  fontGeometry.glyphs.resize(count);

  glm::vec2 calculatedOffset{fontGeometry.totalWidth/2.0f, fontGeometry.totalHeight/2.0f};
  calculatedOffset *= glm::vec2(-1.0f, -1.0f) + preparedString.config().relativeOffset;
  calculatedOffset += preparedString.config().pixelOffset;
  calculatedOffset.y += (fontGeometry.topBearing/2.0f);

  for(auto& glyph : fontGeometry.glyphs)
  {
    for(auto& vert : glyph.vertices)
      vert += calculatedOffset;
  }
}

//##################################################################################################
void FontRenderer::generate()
{
  size_t padding1 = 1;
  size_t padding2 = padding1*2;

  auto pad1 = [padding1](size_t s){return s+padding1;};
  auto pad2 = [padding2](size_t s){return s+padding2;};

  d->glyphGeometry.clear();

  struct GlyphDetails_lt
  {
    //Width and height of the glyph
    size_t width{0};
    size_t height{0};

    float leftBearing  {0.0f}; //Negative for values to the left of 0
    float rightBearing {0.0f}; //Positive to the right of kerningWidth
    float topBearing   {0.0f}; //Positive above 0
    float bottomBearing{0.0f}; //Positive above 0

    float kerningWidth{0.0f};

    std::vector<Pixel> data;

    char16_t character;

    bool valid{false};
  };

  //-- Generate the glyphs -------------------------------------------------------------------------
  std::vector<GlyphDetails_lt*> glyphs;
  for(const auto character : requiredCharacters())
  {
    auto current = new GlyphDetails_lt();
    current->character = character;

    d->font->prepareGlyph(character, [&](const Glyph& glyph)
    {
      modifyGlyph(glyph, [&](const Glyph& glyph)
      {
        size_t size = size_t(glyph.w * glyph.h);
        current->width  = size_t(glyph.w);
        current->height = size_t(glyph.h);

        current->leftBearing   = glyph.leftBearing  ;
        current->rightBearing  = glyph.rightBearing ;
        current->topBearing    = glyph.topBearing   ;
        current->bottomBearing = glyph.bottomBearing;

        current->kerningWidth  = glyph.kerningWidth ;

        current->data.resize(size);
        if(size>0)
          memcpy(current->data.data(), glyph.data, size*sizeof(Pixel));
      });
    });

    glyphs.push_back(current);
  }

  //-- Sort the glyphs by height to aid in box packing ---------------------------------------------
  std::sort(glyphs.begin(), glyphs.end(), [](GlyphDetails_lt* lhs, GlyphDetails_lt* rhs){return lhs->height>rhs->height;});

  //-- Figure out the texture size -----------------------------------------------------------------
  size_t textureSize = 256;
  bool overflow = true;
  for(; textureSize<=4096 && overflow; textureSize*=2)
  {
    size_t w=padding1;
    size_t h=padding1;
    size_t lastHeight=0;

    overflow = false;
    for(const auto glyph : glyphs)
    {
      if(pad2(glyph->width)>textureSize || pad2(glyph->height)>textureSize)
      {
        overflow = true;
        break;
      }

      w+=pad1(glyph->width);

      if(w>textureSize)
      {
        h+=pad1(lastHeight);

        if((h+pad1(glyph->height))>textureSize)
        {
          overflow = true;
          break;
        }

        w=pad1(glyph->width);
      }

      lastHeight = glyph->height;
    }
  }

  //Texture is too large.
  if(overflow)
    return;

  std::vector<Pixel> pixels;
  {
    Pixel p;
    p.r = 0;
    p.g = 0;
    p.b = 0;
    p.a = 0;
    pixels.resize(textureSize*textureSize, p);
  }
  TextureData textureData;
  textureData.w = int(textureSize);
  textureData.h = int(textureSize);
  textureData.data = pixels.data();

  //-- Draw glyphs to the texture ------------------------------------------------------------------
  {
    size_t w=padding1;
    size_t h=padding1;
    size_t lastHeight=0;

    for(const auto glyph : glyphs)
    {
      size_t x = w;

      w+=pad1(glyph->width);

      if(w>textureSize)
      {
        h+=pad1(lastHeight);

        x = padding1;
        w=pad1(glyph->width);
      }

      lastHeight = glyph->height;

      {
        size_t y = h;
        size_t bytes = glyph->width*sizeof(Pixel);
        for(size_t sy=0; sy<glyph->height; sy++, y++)
        {
          const auto src = glyph->data.data() + (sy*glyph->width);
          auto dst = textureData.data + ((y*size_t(textureData.w)) + x);
          memcpy(dst, src, bytes);
        }
      }

      {
        auto& glyphGeometry = d->glyphGeometry[glyph->character];

        float fx = float(x) / float(textureSize);
        float fy = float(h) / float(textureSize);
        float fr = float(x+glyph->width) / float(textureSize);
        float fb = float(h+glyph->height) / float(textureSize);

        glyphGeometry.textureCoords[0] = {fx, fy};
        glyphGeometry.textureCoords[1] = {fr, fy};
        glyphGeometry.textureCoords[2] = {fr, fb};
        glyphGeometry.textureCoords[3] = {fx, fb};

        glyphGeometry.vertices[0] = {               0.0f,                 0.0f+glyph->bottomBearing};
        glyphGeometry.vertices[1] = {float(glyph->width),                 0.0f+glyph->bottomBearing};
        glyphGeometry.vertices[2] = {float(glyph->width), float(glyph->height)+glyph->bottomBearing};
        glyphGeometry.vertices[3] = {               0.0f, float(glyph->height)+glyph->bottomBearing};

        glyphGeometry.leftBearing   = glyph->leftBearing  ;
        glyphGeometry.rightBearing  = glyph->rightBearing ;
        glyphGeometry.topBearing    = glyph->topBearing   ;
        glyphGeometry.bottomBearing = glyph->bottomBearing;

        glyphGeometry.kerningWidth = glyph->kerningWidth;
      }
    }
  }

  //-- Finally assign this to a texture ------------------------------------------------------------
  setTexture(textureData);

  tpDeleteAll(glyphs);
}

//##################################################################################################
void FontRenderer::modifyGlyph(const Glyph& glyph, const std::function<void(const Glyph&)>& addGlyph)
{
  addGlyph(glyph);
}

//##################################################################################################
void FontRenderer::setTexture(const TextureData& texture)
{
  d->texture.setImage(texture);
  d->freeTexture();
  d->bindTexture = true;
}

//##################################################################################################
const std::vector<PreparedString*>& FontRenderer::preparedStrings() const
{
  return d->preparedStrings;
}

//##################################################################################################
const std::unordered_set<char16_t>& FontRenderer::requiredCharacters() const
{
  return d->requiredCharacters;
}

//##################################################################################################
void FontRenderer::addPreparedString(PreparedString* preparedString)
{
  size_t size = d->requiredCharacters.size();

  d->preparedStrings.push_back(preparedString);

  for(const auto character : preparedString->text())
    d->requiredCharacters.insert(character);

  if(size!=d->requiredCharacters.size())
    d->freeAndInvalidate();
}

//##################################################################################################
void FontRenderer::removePreparedString(PreparedString* preparedString)
{
  tpRemoveOne(d->preparedStrings, preparedString);
}

}
