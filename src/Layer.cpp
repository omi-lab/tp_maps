#include "tp_maps/Layer.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Transformation.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{
struct Layer::Private
{
  Layer* q;

  Map* map{nullptr};
  tp_utils::StringID coordinateSystem{defaultSID()};
  tp_math_utils::Transformation transformation;
  bool visible{true};

  std::unordered_map<tp_utils::StringID, std::string> extraDataMap;

  //################################################################################################
  Private(Layer* q_):
    q(q_)
  {

  }
};

//##################################################################################################
Layer::Layer():
  d(new Private(this))
{
}

//##################################################################################################
Layer::~Layer()
{
  if(d->map)
    d->map->mapLayerDestroyed(this);

  delete d;
}

//##################################################################################################
Map* Layer::map()const
{
  return d->map;
}

//##################################################################################################
std::unordered_map<tp_utils::StringID, std::string>& Layer::extraDataMap()
{
  return d->extraDataMap;
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, std::string>& Layer::extraDataMap()const
{
  return d->extraDataMap;
}

//##################################################################################################
void Layer::setCoordinateSystem(const tp_utils::StringID& coordinateSystem)
{
  d->coordinateSystem = coordinateSystem;
}

//##################################################################################################
const tp_utils::StringID& Layer::coordinateSystem()const
{
  return d->coordinateSystem;
}

//##################################################################################################
void Layer::setTransformation(const tp_math_utils::Transformation& transformation)
{
  d->transformation = transformation;
}

//##################################################################################################
const tp_math_utils::Transformation& Layer::transformation()const
{
  return d->transformation;
}

//##################################################################################################
bool Layer::visible() const
{
  return d->visible;
}

//##################################################################################################
void Layer::setVisible(bool visible)
{
  d->visible = visible;
}

//##################################################################################################
void Layer::render(RenderInfo& renderInfo)
{
  TP_UNUSED(renderInfo);
}

//##################################################################################################
void Layer::invalidateBuffers()
{

}

//##################################################################################################
bool Layer::mouseEvent(const MouseEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
void Layer::animate(double timestampMS)
{
  TP_UNUSED(timestampMS);
}

//##################################################################################################
void Layer::update()
{
  if(d->map)
    d->map->update();
}

//##################################################################################################
void Layer::setMap(Map* map)
{
  d->map = map;
}

//##################################################################################################
void Layer::clearMap()
{
  d->map = nullptr;
}
}
