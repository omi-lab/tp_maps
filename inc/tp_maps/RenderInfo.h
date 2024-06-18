#ifndef tp_maps_RenderInfo_h
#define tp_maps_RenderInfo_h

#include "tp_maps/Globals.h"

#include "tp_utils/StringID.h"

#include <functional>
#include <vector>


namespace tp_maps
{
struct PickingDetails;
class RenderInfo;
class PickingResult;
class Layer;
class Map;

//##################################################################################################
//! Picking callback type
/*!
These callbacks will be called when an item is successfully picked, the callback should new up a
PickingResult instance and populate it with any extra information relevant to that type of picking
event. There are subclasses of PickingResult for different types of layer.
*/
typedef std::function<PickingResult*(const PickingResult& result)> PickingCallback;

//##################################################################################################
struct TP_MAPS_EXPORT PickingDetails
{
  //! Should be >=0 and <count
  size_t index;

  //! The method that will be called if this is picked
  PickingCallback callback;

  //! The number of picking ID to reserve
  uint32_t count;

  PickingDetails(size_t index_=0, const PickingCallback& callback_=PickingCallback(), uint32_t count_=1):
    index(index_),
    callback(callback_),
    count(count_)
  {

  }
};

//##################################################################################################
class TP_MAPS_EXPORT RenderInfo
{
public:

  //! The map that the render is happening in
  Map* map;

  //! The render method will get called multiple times for different purposes
  RenderPass pass;

  //! Are we rendering to HDR buffers.
  HDR hdr{HDR::No};

  //! Do we have extra buffers for normals and specular.
  ExtendedFBO extendedFBO{ExtendedFBO::No};

  //! If this is a picking pass this will contain the type of picking request
  /*!
  Picking passes can be used for different purposes for example:
  <ul>
    <li><b>Selection</b> - This should be used for selecting items
    <li><b>Tool tip</b> - This should set the tool tip of the widget
    <li><b>Context menu</b> - Display a context menu
  </ul>
  */
  tp_utils::StringID pickingType;

  //! The position of the mouse pointer, if this is a picking pass
  glm::ivec2 pos;

  //! Holds the list of picking details, this gets reset each time picking is rendered
  std::vector<PickingDetails> pickingDetails;
  uint32_t nextID{1};

  //################################################################################################
  uint32_t pickingID(const PickingDetails& details);

  //################################################################################################
  glm::vec4 pickingIDMat(const PickingDetails& details);

  //################################################################################################
  static uint32_t pickingIDFromColor(uint32_t r, uint32_t g, uint32_t b);

  //################################################################################################
  //! Call this at the start of the picking pass
  void resetPicking();

  //################################################################################################
  ShaderType shaderType() const;
};

}

#endif
