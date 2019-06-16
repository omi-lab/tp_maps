#include "tp_maps/SpriteTexture.h"
#include "tp_maps/Texture.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

//##################################################################################################
SpriteTexture::~SpriteTexture()
{
  delete m_texture;
}

//##################################################################################################
Texture* SpriteTexture::texture()const
{
  return m_texture;
}

//##################################################################################################
const std::vector<SpriteCoords>& SpriteTexture::coords()const
{
  return m_coords;
}

//##################################################################################################
void SpriteTexture::setCoordsChangedCallback(std::function<void()> callback)
{
  m_callback = std::move(callback);
}

//##################################################################################################
void SpriteTexture::setTexture(Texture* texture)
{
  if(m_texture)
    tpWarning() << "SpriteTexture::setTexture Error texture already set!";

  m_texture = texture;
}

//##################################################################################################
void SpriteTexture::setCoords(const std::vector<SpriteCoords>& coords)
{
  m_coords = coords;
  if(m_callback)
    m_callback();
}

}
