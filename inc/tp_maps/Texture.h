#ifndef tp_maps_Texture_h
#define tp_maps_Texture_h

#include "tp_maps/Globals.h"

#include "glm/glm.hpp"

#include <functional>

namespace tp_maps
{
class Map;

//##################################################################################################
//! This is the base class for textures.
/*!
Because tp_maps is platform agnostic it needs to delegate the task of loading images off to some
other part of the application. This class provides an abstract interface that is used bind a
texture and provide a OpenGL texture ID.
*/
class Texture
{
  Map* m_map;
  GLuint m_magFilterOption{GL_LINEAR};
  GLuint m_minFilterOption{GL_LINEAR_MIPMAP_LINEAR};
  std::function<void()> m_callback;
public:
  //################################################################################################
  Texture(Map* map);

  //################################################################################################
  virtual ~Texture();

  //################################################################################################
  void setImageChangedCallback(std::function<void()> callback);

  //################################################################################################
  //! Subclasses should implement this and return true if they have an image to display
  virtual bool imageReady()=0;

  //################################################################################################
  //! This will be called to get the image texture ID it must bind the texture.
  /*!
  Subclasses should implement this and when it is called they should bind their texture.
  \return The texture ID.
  */
  virtual GLuint bindTexture()=0;

  //################################################################################################
  void setMagFilterOption(GLuint magFilterOption);

  //################################################################################################
  void setMinFilterOption(GLuint minFilterOption);

  //################################################################################################
  GLuint magFilterOption()const;

  //################################################################################################
  GLuint minFilterOption()const;

  //################################################################################################
  //! Returns the percentage of the texture that the image occupies
  virtual glm::vec2 textureDims()const;

  //################################################################################################
  //! The size of the image in pixels
  virtual glm::vec2 imageDims()const=0;

  //################################################################################################
  void deleteTexture(GLuint id);

protected:
  //################################################################################################
  Map* map()const;

  //################################################################################################
  //! This should be called by sub classes when the image changes
  void imageChanged();
};

}

#endif
