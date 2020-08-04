#ifndef tp_maps_Layer_h
#define tp_maps_Layer_h

#include "tp_maps/Globals.h"
#include "tp_maps/RenderInfo.h"

#include "tp_utils/StringID.h"
#include "tp_utils/CallbackCollection.h"

#include <unordered_map>

namespace tp_math_utils
{
class Transformation;
}

namespace tp_maps
{
class Map;
class RenderInfo;
struct MouseEvent;
struct KeyEvent;
struct TextEditingEvent;
struct TextInputEvent;

//##################################################################################################
//! A directory of picking callbacks, and a description of how different layers handle picking callbacks.
/*!
\defgroup PickingCallbacks Picking Callbacks

It is possible to add picking callbacks to layers. Each different type of layer handles its
callbacks differently. For example the image layer passed a pointer to the layer as the opaque,
whereas picking callbacks that are passed to the WebLayer should expect a pointer to the
WebLayerItem in the opaque.

This page attempts to document all the different schemes together.
*/

//##################################################################################################
//! The base class for layers in the map.
/*!
Layers are the main way of interacting with the map. They can be used to draw in it, and receive
user events.
*/
class TP_MAPS_SHARED_EXPORT Layer
{
  friend class Map;
public:
  //################################################################################################
  //! Construct the layer.
  /*!
  This will construct the layer, however the QGLFunctions will not be initialized until the layer
  is added to the map.
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
  Map* map()const;

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
  const tp_utils::StringID& coordinateSystem()const;

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

  //################################################################################################
  //! The render pass that this layer should do most of its rendering in.
  /*!
  Layers may use this to select a render pass to do their main drawing in. Many layers will need to
  perform rendering in multiple passes, its up to the layer to do this sensibly.

  \return The default render pass for this layer.
  */
  RenderPass defaultRenderPass() const;

  //################################################################################################
  //! Set the render pass that this layer should do most of its rendering in.
  void setDefaultRenderPass(RenderPass defaultRenderPass);

  //################################################################################################
  tp_utils::CallbackCollection<void(double)> animateCallbacks;

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
  const std::vector<Layer*>& childLayers()const;

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
  virtual bool textEditingEvent(const TextEditingEvent& event);

  //################################################################################################
  virtual bool textInputEvent(const TextInputEvent& event);

  //################################################################################################
  //! Update the state of the animation
  virtual void animate(double timestampMS);

  //################################################################################################
  virtual void lightsChanged(LightingModelChanged lightingModelChanged);

  //################################################################################################
  //! Calls update on the map
  void update();

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

  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
