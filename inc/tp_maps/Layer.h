#ifndef tp_maps_Layer_h
#define tp_maps_Layer_h

#include "tp_maps/Globals.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/MouseEvent.h"

#include "tp_utils/StringID.h"
#include "tp_utils/CallbackCollection.h"

#include <unordered_map>
#include <unordered_set>

namespace tp_math_utils
{
class Transformation;
}

namespace tp_maps
{
class Map;
class LayerPointer;
class RenderInfo;
struct KeyEvent;
struct DragDropEvent;
struct TextEditingEvent;
struct TextInputEvent;

//##################################################################################################
//! The base class for layers in the map.
/*!
Layers are the main way of interacting with the map. They can be used to draw in it, and receive
user events.
*/
class TP_MAPS_EXPORT Layer
{
  friend class Map;
  friend class LayerPointer;
  TP_NONCOPYABLE(Layer);
  TP_DQ;
public:
  //################################################################################################
  //! Construct the layer.
  /*!
  This will construct the layer.
  */
  Layer();

  //################################################################################################
  //! Destructor.
  /*!
  All child layers of this layer are deleted.
  */
  virtual ~Layer();

  //################################################################################################
  //! Returns the Map that this layer is in.
  /*!
  After the layer has been added to a map this will return a pointer to the map, until then
  it will return nullptr.

  \return A pointer to the map that this layer is in or nullptr.
  */
  Map* map() const;

  //################################################################################################
  //! Returns the parent layer or nullptr.
  Layer* parentLayer() const;

  //################################################################################################
  //! Transforms from model coords to parent coords
  /*!
  This if the matrix that transforms this layers coords into its parents coordinate system, if this
  layer has a parentLayer this will transform to the coordinate system of parentLayer else it
  transforms into the maps coordinate system aka world coords.

  If you want to transform from model to world coords use modelToWorldMatrix this will multiply all
  parent matricies for you.

  \return The model matrix.
  */
  const glm::mat4& modelMatrix() const;

  //################################################################################################
  void setModelMatrix(const glm::mat4& modelMatrix, bool requestUpdate=true);

  //################################################################################################
  //! Returns a matrix that transforms from this models coordinate system to world coords.
  /*!
  This will multiply the model matricies of this layer and all its parents to produce a matrix that
  transforms model coords into world coords.

  \return The model to world matrix for this layer.
  */
  glm::mat4 modelToWorldMatrix() const;

  //################################################################################################
  //! Sets the coordinate system that this layer uses
  /*!
  A map can be configured to use multiple coordinate systems. See \link
  Map::cameraMatrix(const tp_utils::StringID&) \endlink for further details.

  \param coordinateSystem - The name of the coordinate system to use.
  */
  void setCoordinateSystem(const tp_utils::StringID& coordinateSystem);

  //################################################################################################
  //! Returns the coordinate system that this layer should use
  /*!
  \sa setCoordinateSystem()
  \return The coordinate system that this layer should use.
  */
  const tp_utils::StringID& coordinateSystem() const;

  //################################################################################################
  //! Returns the visibility of the layer
  /*!
  \return The visibility of this layer
  \sa setVisible()
   */
  bool visible() const;

  //################################################################################################
  //! Set the visibility of the layer
  /*!
  Setting the visibility to false explicitly hides the layer. Setting it to true will make the layer
  visible if all of the layers above it in the hierarchy are also visible.

  \param visible: the new visibility for the layer
  */
  virtual void setVisible(bool visible);
  void setVisibleQuiet(bool visible);

  //################################################################################################
  bool visibileToCurrentSubview() const;

  //################################################################################################
  bool excludeFromPicking() const;

  //################################################################################################
  void setExcludeFromPicking(bool excludeFromPicking);

  //################################################################################################
  //! The render pass that this layer should do most of its rendering in.
  /*!
  Layers may use this to select a render pass to do their main drawing in. Many layers will need to
  perform rendering in multiple passes, its up to the layer to do this sensibly.

  \return The default render pass for this layer.
  */
  const RenderPass& defaultRenderPass() const;

  //################################################################################################
  //! Set the render pass that this layer should do most of its rendering in.
  virtual void setDefaultRenderPass(const RenderPass& defaultRenderPass);

  //################################################################################################
  //! The names of subviews to exclude this layer from.
  const std::unordered_set<tp_utils::StringID>& excludeFromSubviews() const;

  //################################################################################################
  void setExcludeFromSubviews(const std::unordered_set<tp_utils::StringID>& excludeFromSubviews);

  //################################################################################################
  //! Called when buffers become invalid.
  /*!
  This is called when the OpenGL context becomes invalid, all OpenGL resources should be ignored.
  */
  tp_utils::CallbackCollection<void()> invalidateBuffersCallbacks;

  //################################################################################################
  //! Add a child layer
  /*!
  The default implementation simply appends the layer to the vector of layers and then calls
  setMap() on the layer.

  \note The layer will take ownership of the child layer.

  \param layer The child layer to add.
  */
  virtual void addChildLayer(Layer* layer);

  //################################################################################################
  //! Insert a child layer at a given position
  void insertChildLayer(size_t i, Layer* layer);

  //################################################################################################
  //! Remove a child layer
  /*!
  \param layer The child layer to remove
  */
  void removeChildLayer(Layer* layer);

  //################################################################################################
  //! Remove and delete all child layers
  void clearChildLayers();

  //################################################################################################
  //! Return the list of child layers
  const std::vector<Layer*>& childLayers() const;

protected:
  //################################################################################################
  //! Return the list of child layers
  std::vector<Layer*>& childLayers();

  //################################################################################################
  //! This is called to get the layer to render.
  /*!
  This should be sub-classed to perform any drawing that the layer should do. This will be called
  multiple times to perform different types of rendering. See the \link RenderInfo \endlink
  documentation for a complete description of how to handle the different render passes.

  \param renderInfo - Details about the type of render to perform.
  */
  virtual void render(RenderInfo& renderInfo);

  //################################################################################################
  //! Called when buffers become invalid.
  /*!
  This is called when the OpenGL context becomes invalid, all OpenGL resources should be ignored.
  */
  virtual void invalidateBuffers();

  //################################################################################################
  //! Called with mouse events.
  /*!
  \param event - Details of the event that was passed to the map.
  */
  virtual bool mouseEvent(const MouseEvent& event);

  //################################################################################################
  //! Called with key events.
  /*!
  \param event - Details of the event that was passed to the map.
  */
  virtual bool keyEvent(const KeyEvent& event);

  //################################################################################################
  //! Called with DragDrop events.
  /*!
  \param event - Details of the event that was passed to the map.
  */
  virtual bool dragDropEvent(const DragDropEvent& event);

  //################################################################################################
  virtual bool textEditingEvent(const TextEditingEvent& event);

  //################################################################################################
  virtual bool textInputEvent(const TextInputEvent& event);

  //################################################################################################
  //! Update the state of the animation
  virtual void animate(double timestampMS);

  //################################################################################################
  virtual void lightsChanged(LightingModelChanged lightingModelChanged);

  //################################################################################################
  //! Called by the map when it is resized
  virtual void mapResized(int w, int h);

  //################################################################################################
  //! Calls update on the map
  void update(RenderFromStage renderFromStage=RenderFromStage::Full);

  //################################################################################################
  void callAsync(const std::function<void()>& callback);

  //################################################################################################
  virtual void addedToMap();

private:  
  //################################################################################################
  //! Called when a child layer is deleted.
  void childLayerDestroyed(Layer* layer);

  //################################################################################################
  //! Called by the Map.
  void setMap(Map* map, Layer* parent);

  //################################################################################################
  //! Called by the Map.
  void clearMap();

  //################################################################################################
  void addPointer(LayerPointer* layerPointer);

  //################################################################################################
  void removePointer(LayerPointer* layerPointer);

  std::unordered_set<Button> m_hasMouseFocusFor; //!< Set when this layer accepts focus for a mouse press event.
  std::unordered_set<int32_t> m_hasKeyFocusFor;  //!< Set when this layer accepts focus for a key press event
};

}

#endif
