#ifndef tp_maps_Map_h
#define tp_maps_Map_h

#include "tp_maps/Globals.h"

#include "glm/glm.hpp"

#include <string>
#include <vector>

namespace tp_math_utils
{
class Plane;
}

namespace tp_utils
{
class StringID;
}

namespace tp_maps
{
class Layer;
class Controller;
class RenderInfo;
class Shader;
class Texture;
struct MouseEvent;
class PickingResult;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Map
{  
  friend class Layer;
  friend class Controller;

public:
  //################################################################################################
  Map(bool enableDepthBuffer = true);

  //################################################################################################
  virtual ~Map();  

protected:
  //################################################################################################
  void preDelete();

public:

  //################################################################################################
  bool initialized()const;

  //################################################################################################
  //! Print OpenGL errors
  /*!
  \param description - This will be printed with the OpenGL error.
  */
  static void printOpenGLError(const std::string& description);

  //################################################################################################
  //!Sets the background clear color
  void setBackgroundColor(const glm::vec3& color);

  //################################################################################################
  //! Returns the background clear color
  glm::vec3 backgroundColor()const;

  //################################################################################################
  void setEnableDepthBuffer(bool enableDepthBuffer);

  //################################################################################################
  bool enableDepthBuffer()const;

  //################################################################################################
  //! Add a layer to the map
  /*!
  The default implementation simply appends the layer to the vector of layers and then
  calls setMap() on the layer.

  \note The map will take ownership of the layer.

  \param layer The layer to add to the map.
  */
  virtual void addLayer(Layer* layer);

  //################################################################################################
  //! Insert a layer at a given position
  void insertLayer(int i, Layer* layer);

  //################################################################################################
  //! Remove a layer from the map
  /*!
  \param layer The layer to remove from the map
  */
  void removeLayer(Layer* layer);

  //################################################################################################
  //! Remove and delete all layers from the map
  void clearLayers();

  //################################################################################################
  //! Return the vector of map layers
  const std::vector<Layer*>& layers()const;

protected:
  //################################################################################################
  //! Return the list of map layers
  std::vector<Layer*>& layers();

public:

  //################################################################################################
  void project(const glm::vec3& scenePoint, glm::vec2& screenPoint, const glm::mat4& matrix);

  //################################################################################################
  bool unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane);

  //################################################################################################
  bool unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane, const glm::mat4& matrix);

  //################################################################################################
  glm::vec3 unProject(const glm::vec3& screenPoint);

  //################################################################################################
  glm::vec3 unProject(const glm::vec3& screenPoint, const glm::mat4& matrix);

  //################################################################################################
  //! Performs a picking render
  /*!
  This will perform a picking render and return the results, the picked layer may return a subclass
  of PickingResult that contains extra information. If nothing is picked this method will return
  nullptr, also the picking function that is called may decide to return nullptr if it wishes.

  \param pickingType - The type of this picking pass, this effects what layers do with the result.
  \param pos - The position on screen to perform the picking.
  \return A pointer to a picking result or nullptr, the caller must delete this.
  */
  PickingResult* performPicking(const tp_utils::StringID& pickingType, const glm::ivec2& pos);

  //################################################################################################
  //! Delete the given texture
  /*!
  \param id: The id of the texture to delete
  */
  void deleteTexture(GLuint id);

  //################################################################################################
  //! Use this to allocate your shaders
  /*!
  This will either return an existing shader created in a previous call or create a new shader and
  add it to the shader. This allows shaders to be shared between layers.
  */
  template<typename T>
  T* getShader()
  {
    const tp_utils::StringID& name = T::name();
    T* shader = static_cast<T*>(getShader(name));
    if(!shader)
    {
      shader = new T();
      addShader(name, shader);
    }
    return shader;
  }

  //################################################################################################
  //! Return the map's window width
  /*!
  \return the width of the window
  */
  int width() const;

  //################################################################################################
  //! Return the map's window height
  /*!
  \return the height of the window
  */
  int height() const;

  //################################################################################################
  glm::vec2 screenSize()const;

  //################################################################################################
  //! Make the GL context of this map current
  virtual void makeCurrent() = 0;

  //################################################################################################
  //! Called to queue a refresh
  virtual void update() = 0;

  //################################################################################################
  void initializeGL();

  //################################################################################################
  void paintGL();

  //################################################################################################
  void resizeGL(int w, int h);

  //################################################################################################
  bool mouseEvent(const MouseEvent& event);

  //################################################################################################
  //! Return the controller
  Controller* controller();

  //################################################################################################
  //! Return the render info
  RenderInfo& renderInfo();

  //################################################################################################
  //! Update the state of the animation
  virtual void animate(double timestampMS);

private:
  //################################################################################################
  //! Called by the Layer when it is destroyed
  void mapLayerDestroyed(Layer* layer);

  //################################################################################################
  //! New Controller's add them selves to the MapWidget replacing existing controllers
  void setController(Controller* controller);

  //################################################################################################
  //! Return the shader for name or nullptr if it does not exist
  Shader* getShader(const tp_utils::StringID& name)const;

  //################################################################################################
  //! Add a shader to the map of shaders
  void addShader(const tp_utils::StringID& name, Shader* shader);

  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif
