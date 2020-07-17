#include "tp_maps/Layer.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Transformation.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{
struct Layer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Layer::Private");
  TP_NONCOPYABLE(Private);

  Layer* q;

  Map* map{nullptr};
  tp_utils::StringID coordinateSystem{defaultSID()};
  RenderPass defaultRenderPass{RenderPass::Normal};
  bool visible{true};

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
bool Layer::visible() const
{
  return d->visible;
}

//##################################################################################################
void Layer::setVisible(bool visible)
{
  d->visible = visible;
  update();
}

//##################################################################################################
RenderPass Layer::defaultRenderPass() const
{
  return d->defaultRenderPass;
}

//##################################################################################################
void Layer::setDefaultRenderPass(RenderPass defaultRenderPass)
{
  d->defaultRenderPass = defaultRenderPass;
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
bool Layer::keyEvent(const KeyEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
bool Layer::textEditingEvent(const TextEditingEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
bool Layer::textInputEvent(const TextInputEvent& event)
{
  TP_UNUSED(event);
  return false;
}

//##################################################################################################
void Layer::animate(double timestampMS)
{
  animateCallbacks(timestampMS);
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
