#include "tp_maps/Texture.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
Texture::Texture(Map* map):
  m_map(map)
{

}

//##################################################################################################
Texture::~Texture() = default;

//##################################################################################################
void Texture::setImageChangedCallback(std::function<void()> callback)
{
  m_callback = std::move(callback);
}

//##################################################################################################
void Texture::setMagFilterOption(GLint magFilterOption)
{
  m_magFilterOption = magFilterOption;
}

//##################################################################################################
void Texture::setMinFilterOption(GLint minFilterOption)
{
  m_minFilterOption = minFilterOption;
}

//##################################################################################################
void Texture::setTextureWrapS(GLint textureWrapS)
{
  m_textureWrapS = textureWrapS;
}

//##################################################################################################
void Texture::setTextureWrapT(GLint textureWrapT)
{
  m_textureWrapT = textureWrapT;
}

//##################################################################################################
GLint Texture::magFilterOption() const
{
  return m_magFilterOption;
}

//##################################################################################################
GLint Texture::minFilterOption() const
{
  return m_minFilterOption;
}

//##################################################################################################
GLint Texture::textureWrapS() const
{
  return m_textureWrapS;
}

//##################################################################################################
GLint Texture::textureWrapT() const
{
  return m_textureWrapT;
}

//##################################################################################################
glm::vec2 Texture::textureDims() const
{
  return {1.0f, 1.0f};
}

//##################################################################################################
void Texture::deleteTexture(GLuint id)
{
  m_map->deleteTexture(id);
}

//##################################################################################################
Map* Texture::map() const
{
  return m_map;
}

//##################################################################################################
void Texture::imageChanged()
{
  if(m_callback)
    m_callback();
}

}
