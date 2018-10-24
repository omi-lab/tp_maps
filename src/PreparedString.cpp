#include "tp_maps/PreparedString.h"
#include "tp_maps/Font.h"
#include "tp_maps/FontRenderer.h"

namespace tp_maps
{

//##################################################################################################
struct PreparedString::Private
{
  FontRenderer* fontRenderer;
  const std::u16string& text;
  FontGeometry fontGeometry;

  bool regenerateBuffers{true};
  bool topDown;

  //################################################################################################
  Private(FontRenderer* fontRenderer_, const std::u16string& text_, bool topDown_):
    fontRenderer(fontRenderer_),
    text(text_),
    topDown(topDown_)
  {

  }
};

//##################################################################################################
PreparedString::PreparedString(FontRenderer* fontRenderer, const std::u16string& text, bool topDown):
  d(new Private(fontRenderer, text, topDown))
{
  d->fontRenderer->addPreparedString(this);
}

//##################################################################################################
PreparedString::~PreparedString()
{
  d->fontRenderer->removePreparedString(this);
  delete d;
}

//################################################################################################
const std::u16string& PreparedString::text() const
{
  return d->text;
}

//##################################################################################################
GLuint PreparedString::textureID() const
{
  return d->fontRenderer->textureID();
}

//##################################################################################################
const FontGeometry& PreparedString::fontGeometry() const
{
  if(d->regenerateBuffers)
  {
    d->regenerateBuffers = false;
    d->fontRenderer->prepareFontGeometry(*this, d->fontGeometry);
  }

  return d->fontGeometry;
}

//##################################################################################################
void PreparedString::invalidateBuffers()
{

}

//##################################################################################################
void PreparedString::regenerateBuffers()
{
  d->regenerateBuffers = true;
}

//##################################################################################################
bool PreparedString::topDown() const
{
  return d->topDown;
}

}
