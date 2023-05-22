#ifndef tp_maps_Map_h
#define tp_maps_Map_h

#include "tp_maps/Globals.h"

#include "tp_image_utils/ColorMap.h"
#include "tp_image_utils/ColorMapF.h"

#include "tp_math_utils/Light.h"

#include "tp_utils/CallbackCollection.h"

#include <string>
#include <vector>

namespace tp_math_utils
{
class Plane;
}

namespace tp_utils
{
class StringID;
class Profiler;
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
class Buffers;
class PostLayer;

//##################################################################################################
class TP_MAPS_EXPORT Map
{
  friend class Layer;
  friend class Controller;
  friend class FontRenderer;
  TP_NONCOPYABLE(Map);

public:
  //################################################################################################
  Map(bool enableDepthBuffer = false, tp_utils::Profiler* profiler = nullptr);

  //################################################################################################
  virtual ~Map();

protected:
  //################################################################################################
  void preDelete();

  //################################################################################################
  //! Only ever call this before any calls to render.
  void setOpenGLProfile(OpenGLProfile openGLProfile);

  //################################################################################################
  void setVisible(bool visible);
public:

  //################################################################################################
  OpenGLProfile openGLProfile() const;

  //################################################################################################
  //! Returns true if the 3D view is currently visible
  /*!
  Exactly what this means is patform dependent, in a desktop application this is true if the widget
  is currently on screen and false if the widget is for example in a tab that is not currently
  selected.

  \return True if the 3D view is currently visible
  */
  bool visible() const;

  //################################################################################################
  bool initialized() const;

  //################################################################################################
  const Buffers& buffers() const;

  //################################################################################################
  //!Sets the background clear color
  void setBackgroundColor(const glm::vec3& color);

  //################################################################################################
  //!Sets the background clear color
  void setBackgroundColor(const glm::vec4& color);

  //################################################################################################
  //! Returns the background clear color
  glm::vec3 backgroundColor() const;

  //################################################################################################
  void setWriteAlpha(GLboolean writeAlpha);

  //################################################################################################
  //! Should we write to the alpha channel of the render buffer.
  GLboolean writeAlpha() const;

  //################################################################################################
  void setRenderPasses(const std::vector<RenderPass>& renderPasses);

  //################################################################################################
  void setLights(const std::vector<tp_math_utils::Light>& lights);

  //################################################################################################
  const std::vector<tp_math_utils::Light>& lights() const;

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
  tp_utils::CallbackCollection<void(size_t, Layer*)> layerInserted;

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
  const std::vector<Layer*>& layers() const;

  //################################################################################################
  template<typename T>
  void findLayers(const std::function<void(T*)>& closure)
  {
    for(auto l : layers())
      if(auto ll = dynamic_cast<T*>(l); ll)
        closure(ll);
  }

  //################################################################################################
  //! Reset to teh default controller.
  void resetController();

  //################################################################################################
  void setMaxLightTextureSize(size_t maxLightTextureSize);

  //################################################################################################
  size_t maxLightTextureSize() const;

  //################################################################################################
  void setMaxSamples(size_t maxSamples);

  //################################################################################################
  size_t maxSamples() const;

  //################################################################################################
  void setMaxSpotLightLevels(size_t maxSpotLightLevels);

  //################################################################################################
  size_t maxSpotLightLevels() const;

  //################################################################################################
  size_t renderedLightLevels() const;

  //################################################################################################
  void setMaxLightRenderTime(size_t maxLightRenderTime);

  //################################################################################################
  size_t maxLightRenderTime() const;

  //################################################################################################
  void setShadowSamples(size_t shadowSamples);

  //################################################################################################
  //! The number of adjacent samples to take from a shadow texture
  /*!
  The actual number of samples that is made is as follows:

  samples = (shadowSamples+1) * (shadowSamples+1) * spotLightLevels * nLights;
  */
  size_t shadowSamples() const;

  //################################################################################################
  //! Enable high dynamic range rendering buffers.
  void setHDR(HDR hdr);

  //################################################################################################
  //! Returns true if HDR is enabled and supported.
  HDR hdr() const;

  //################################################################################################
  //! Enable deferred rendering buffers.
  void setExtendedFBO(ExtendedFBO extendedFBO);

  //################################################################################################
  //! Returns true if extended rendering buffers are enabled.
  ExtendedFBO extendedFBO() const;

  //################################################################################################
  //! Called when buffers become invalid.
  /*!
  This is called when the OpenGL context becomes invalid, all OpenGL resources should be ignored.
  */
  tp_utils::CallbackCollection<void()> invalidateBuffersCallbacks;

  //################################################################################################
  //! Called each time the controller triggers an update.
  /*!
  This is usually trigged as a result of the view changing for example the camera moving.
  */
  tp_utils::CallbackCollection<void()> controllerUpdate;

  //################################################################################################
  tp_utils::CallbackCollection<void(double)> animateCallbacks;

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
  \param image this will be resized and the results will be written to here.
  \param swapY swap rows to convert OpenGL format images to normal images.
  \return True if successful.
  */
  bool renderToImage(size_t width, size_t height, tp_image_utils::ColorMap& image, bool swapY=true);

  //################################################################################################
  bool renderToImage(size_t width, size_t height, TPPixel* pixels, bool swapY=true);

  //################################################################################################
  //! Resize the view and render to an image.
  /*!
  This will create a frame buffer of width x height dimensions and render the map to it, if
  successful the results will be written to pixels and this will return true. This method should
  return the map to its original state when it is done.

  This renders using fully HDR buffers if possible.

  \param width of image to render.
  \param height of image to render.
  \param image this will be resized and the results will be written to here.
  \param swapY swap rows to convert OpenGL format images to normal images.
  \return True if successful.
  */
  bool renderToImage(size_t width, size_t height, tp_image_utils::ColorMapF& image, bool swapY=true);

  //################################################################################################
  //! Resize the view and render to an image.
  bool renderToImage(size_t width, size_t height, HDR hdr, const std::function<void()>& renderComplete);


  //################################################################################################
  //! Delete the given texture
  /*!
  \param id: The id of the texture to delete
  */
  void deleteTexture(GLuint id);

  //################################################################################################
  //! Use this to allocate your shaders.
  /*!
  This will either return an existing shader created in a previous call or create a new shader and
  add it to the shader. This allows shaders to be shared between layers.
  */
  template<typename T, typename... Args>
  T* getShader(Args... args)
  {
    const tp_utils::StringID& name = T::name();
    T* shader = static_cast<T*>(getShader(name));
    if(!shader)
    {
      shader = new T(this, openGLProfile(), args...);
      addShader(name, shader);
    }
    return shader;
  }

  //################################################################################################
  //! Use this to delete shaders to force a recompile, no need to delete shader once you are done.
  void deleteShader(const tp_utils::StringID& name);

  //################################################################################################
  const FBO& currentReadFBO();

  //################################################################################################
  const FBO& currentDrawFBO();

  //################################################################################################
  //! Returns the depth textures for each light.
  const std::vector<FBO>& lightBuffers() const;

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
  virtual void update(RenderFromStage renderFromStage=RenderFromStage::Full);

  //################################################################################################
  virtual void callAsync(const std::function<void()>& callback)=0;

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

  //################################################################################################
  double timeSincePreviousAnimate() const;

protected:
  //################################################################################################
  //! Used by make current to detect when we are in a paint event and to detect nested paint events.
  Map* inPaint() const;

  //################################################################################################
  void setInPaint(bool inPaint);

  //################################################################################################
  void invalidateBuffers();

  //################################################################################################
  size_t skipRenderPasses();

  //################################################################################################
  void executeRenderPasses(size_t rp, GLint& originalFrameBuffer, bool renderMoreLights);

private:
  //################################################################################################
  //! Called by the Layer when it is destroyed
  void layerDestroyed(Layer* layer);

  //################################################################################################
  //! New Controller's add them selves to the MapWidget replacing existing controllers
  void setController(Controller* controller);

  //################################################################################################
  //! Return the shader for name or nullptr if it does not exist
  Shader* getShader(const tp_utils::StringID& name) const;

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
