#include "tp_maps/PreparedString.h"
#include "tp_maps/Font.h"
#include "tp_maps/FontRenderer.h"

namespace tp_maps
{

//##################################################################################################
struct PreparedString::Private
{
  FontRenderer* fontRenderer;
  std::u16string text;
  FontGeometry fontGeometry;

  PreparedStringConfig config;

  bool regenerateBuffers{true};  

  //################################################################################################
  Private(FontRenderer* fontRenderer_, std::u16string text_, const PreparedStringConfig& config_):
    fontRenderer(fontRenderer_),
    text(std::move(text_)),
    config(config_)
  {

  }
};

//##################################################################################################
PreparedString::PreparedString(FontRenderer* fontRenderer, const std::u16string& text, const PreparedStringConfig& config):
  d(new Private(fontRenderer, text, config))
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
const PreparedStringConfig& PreparedString::config() const
{
  return d->config;
}

}
