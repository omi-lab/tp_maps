#ifndef tp_maps_Map_h
#define tp_maps_Map_h

#include "tp_maps/Globals.h"
#include "tp_maps/textures/BasicTexture.h"

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
struct KeyEvent;
struct TextEditingEvent;
struct TextInputEvent;
class PickingResult;
class FontRenderer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Map
{  
  friend class Layer;
  friend class Controller;
  friend class FontRenderer;

public:
  //################################################################################################
  Map(bool enableDepthBuffer = false);

  //################################################################################################
  virtual ~Map();  

protected:
  //################################################################################################
  void preDelete();

  //################################################################################################
  //! Only ever call this before any calls to render.
  void setOpenGLProfile(OpenGLProfile openGLProfile);

public:

  //################################################################################################
  OpenGLProfile openGLProfile() const;

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
  void setRenderPasses(const std::vector<RenderPass>& renderPasses);

  //################################################################################################
  const std::vector<RenderPass>& renderPasses() const;

  //################################################################################################
  //! Sets the callbacks that are used to configure custom render passes.
  /*!
  \param renderPass The pass to set callbacks for, either: Custom1, Custom2, Custom3, Custom4
  \param start A callback called before performing a render pass used to configure OpenGL.
  \param end A callback called after performing a render pass.
  */
  void setCustomRenderPass(RenderPass renderPass,
                           const std::function<void(RenderInfo&)>& start,
                           const std::function<void(RenderInfo&)>& end=std::function<void(RenderInfo&)>());

  //################################################################################################
  void setLights(const std::vector<Light>& lights);

  //################################################################################################
  const std::vector<Light>& lights()const;

  //################################################################################################
  //! This will take ownership.
  void setSpotLightTexture(Texture* spotLightTexture);

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
  void insertLayer(size_t i, Layer* layer);

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
  //! Return the list of map layers
  const std::vector<Layer*>& layers()const;

protected:
  //################################################################################################
  //! Return the list of map layers
  std::vector<Layer*>& layers();

public:

  //################################################################################################
  //! Project 3D coord to 2D UI coord.
  void project(const glm::vec3& scenePoint, glm::vec2& screenPoint, const glm::mat4& matrix);

  //################################################################################################
  //! Same as project except the 2D coordinate will have its y coordinate in OpenGL coords.
  void projectGL(const glm::vec3& scenePoint, glm::vec2& screenPoint, const glm::mat4& matrix);

  //################################################################################################
  bool unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane);

  //################################################################################################
  bool unProject(const glm::dvec2& screenPoint, glm::dvec3& scenePoint, const tp_math_utils::Plane& plane);

  //################################################################################################
  bool unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane, const glm::mat4& matrix);

  //################################################################################################
  bool unProject(const glm::dvec2& screenPoint, glm::dvec3& scenePoint, const tp_math_utils::Plane& plane, const glm::dmat4& matrix);

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
  //! Resize the view and render to an image.
  /*!
  This will create a frame buffer of width x height dimensions and render the map to it, if
  successful the results will be written to pixels and this will return true. This method should
  return the map to its original state when it is done.

  \param width of image to render.
  \param height of image to render.
  \param pixels this will be resized and the results will be written to here.
  \param swapY swap rows to convert OpenGL format images to normal images.
  \return True if successful.
  */
  bool renderToImage(size_t width, size_t height, std::vector<TPPixel>& pixels, bool swapY=true);

  //################################################################################################
  bool renderToImage(size_t width, size_t height, TPPixel* pixels, bool swapY=true);

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
      shader = new T(this, openGLProfile());
      addShader(name, shader);
    }
    return shader;
  }

  //################################################################################################
  //! If there is a reflection texture this returns its ID.
  GLuint reflectionTexture() const;

  //################################################################################################
  //! If there is a reflection depth texture this returns its ID.
  GLuint reflectionDepth() const;

  //################################################################################################
  //! Returns the depth textures for each light.
  const std::vector<FBO>& lightTextures() const;

  //################################################################################################
  //! If there is a spot light texture this returns its ID.
  /*!
  This texture is used to shape the spot lights; typically a spot light is circular and  bright in
  the middle with a fade around the outside. The texture may contain several paterns the one that is
  selected is controled by the uv coordinates of the light.

  \return OpenGL texture ID for the spot light texture.
  */
  GLuint spotLightTexture() const;

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
  glm::vec2 screenSize() const;

  //################################################################################################
  //! Make the GL context of this map current
  virtual void makeCurrent() = 0;

  //################################################################################################
  //! Called to queue a refresh
  virtual void update() = 0;

  //################################################################################################
  virtual float pixelScale();

  //################################################################################################
  void initializeGL();

  //################################################################################################
  void paintGL();

  //################################################################################################
  void paintGLNoMakeCurrent();

  //################################################################################################
  void resizeGL(int w, int h);

  //################################################################################################
  bool mouseEvent(const MouseEvent& event);

  //################################################################################################
  bool keyEvent(const KeyEvent& event);

  //################################################################################################
  bool textEditingEvent(const TextEditingEvent& event);

  //################################################################################################
  bool textInputEvent(const TextInputEvent& event);

  //################################################################################################
  virtual void setRelativeMouseMode(bool enabled);

  //################################################################################################
  virtual bool relativeMouseMode() const;

  //################################################################################################
  virtual void startTextInput();

  //################################################################################################
  virtual void stopTextInput();

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
  void layerDestroyed(Layer* layer);

  //################################################################################################
  //! New Controller's add them selves to the MapWidget replacing existing controllers
  void setController(Controller* controller);

  //################################################################################################
  //! Return the shader for name or nullptr if it does not exist
  Shader* getShader(const tp_utils::StringID& name)const;

  //################################################################################################
  //! Add a shader to the map of shaders
  void addShader(const tp_utils::StringID& name, Shader* shader);

  //################################################################################################
  void addFontRenderer(FontRenderer* fontRenderer);

  //################################################################################################
  void removeFontRenderer(FontRenderer* fontRenderer);

  struct Private;
  Private* d;
  friend struct Private;
};
}
#endif
