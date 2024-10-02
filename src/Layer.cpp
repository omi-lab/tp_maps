#include "tp_maps/Layer.h"
#include "tp_maps/LayerPointer.h"
#include "tp_maps/Map.h"
#include "tp_maps/Subview.h"
#include "tp_maps/DragDropEvent.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/TimeUtils.h"

namespace tp_maps
{
//##################################################################################################
struct Layer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Layer::Private");
  TP_NONCOPYABLE(Private);

  Layer* q;

  std::vector<LayerPointer*> layerPointers;

  Map* map{nullptr};
  Layer* parent{nullptr};
  std::vector<Layer*> layers;
  glm::mat4 modelMatrix{1.0f};
  tp_utils::StringID coordinateSystem{defaultSID()};
  RenderPass defaultRenderPass{RenderPass::Normal};

  std::unordered_set<tp_utils::StringID> excludeFromSubviews;
  std::vector<tp_utils::StringID> onlyInSubviews;
  bool inheritSubviews{true};

  bool visible{true};
  bool excludeFromPicking{false};
  std::shared_ptr<int> alive{std::make_shared<int>()};

  //################################################################################################
  Private(Layer* q_):
    q(q_)
  {

  }

  //################################################################################################
  void propagateSubviews()
  {
    for(const auto& layer : layers)
    {
      if(!layer->d->inheritSubviews)
        continue;

      layer->d->excludeFromSubviews = excludeFromSubviews;
      layer->d->onlyInSubviews = onlyInSubviews;
      layer->d->propagateSubviews();
    }
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
  for(LayerPointer* layerPointer : d->layerPointers)
    layerPointer->m_layer = nullptr;

  clearChildLayers();

  if(d->parent)
    d->parent->childLayerDestroyed(this);
  else if(d->map)
    d->map->layerDestroyed(this);

  delete d;
}

//##################################################################################################
Map* Layer::map() const
{
  return d->map;
}

//##################################################################################################
Layer* Layer::parentLayer() const
{
  return d->parent;
}

//##################################################################################################
const glm::mat4& Layer::modelMatrix() const
{
  return d->modelMatrix;
}

//##################################################################################################
void Layer::setModelMatrix(const glm::mat4& modelMatrix, bool requestUpdate)
{
  d->modelMatrix = modelMatrix;

  if(requestUpdate)
    update();
}

//##################################################################################################
glm::mat4 Layer::modelToWorldMatrix() const
{
  glm::mat4 m=d->modelMatrix;

  for(Layer* p=d->parent; p; p = p->d->parent)
    m = p->d->modelMatrix * m;

  return m;
}

//##################################################################################################
void Layer::setCoordinateSystem(const tp_utils::StringID& coordinateSystem)
{
  d->coordinateSystem = coordinateSystem;
}

//##################################################################################################
const tp_utils::StringID& Layer::coordinateSystem() const
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
  if(d->visible != visible)
  {
    d->visible = visible;
    update();
  }
}

//##################################################################################################
void Layer::setVisibleQuiet(bool visible)
{
  d->visible = visible;
}

//##################################################################################################
bool Layer::visibileToCurrentSubview() const
{
  if(!d->visible)
    return false;

  if(!d->onlyInSubviews.empty() && !tpContains(d->onlyInSubviews, map()->currentSubview()->name()))
    return false;

  if(tpContains(d->excludeFromSubviews, map()->currentSubview()->name()))
    return false;

  return true;
}

//##################################################################################################
Subview* Layer::subview() const
{
  if(!d->onlyInSubviews.empty())
  {
    for(auto subview : map()->allSubviews())
      if(tpContains(d->onlyInSubviews, subview->name()))
        return subview;
  }

  else if(!d->excludeFromSubviews.empty())
  {
    for(auto subview : map()->allSubviews())
      if(!tpContains(d->excludeFromSubviews, subview->name()))
        return subview;
  }

  return map()->defaultSubview();
}

//##################################################################################################
bool Layer::excludeFromPicking() const
{
  return d->excludeFromPicking;
}

//##################################################################################################
void Layer::setExcludeFromPicking(bool excludeFromPicking)
{
  d->excludeFromPicking = excludeFromPicking;
}

//##################################################################################################
const RenderPass& Layer::defaultRenderPass() const
{
  return d->defaultRenderPass;
}

//##################################################################################################
void Layer::setDefaultRenderPass(const RenderPass& defaultRenderPass)
{
  d->defaultRenderPass = defaultRenderPass;
}

//##################################################################################################
const std::unordered_set<tp_utils::StringID>& Layer::excludeFromSubviews() const
{
  return d->excludeFromSubviews;
}

//##################################################################################################
void Layer::setExcludeFromSubviews(const std::unordered_set<tp_utils::StringID>& excludeFromSubviews)
{
  d->excludeFromSubviews = excludeFromSubviews;
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
void Layer::setExcludeFromSubviews(const std::vector<tp_utils::StringID>& excludeFromSubviews)
{
  d->excludeFromSubviews.clear();
  d->excludeFromSubviews.reserve(excludeFromSubviews.size());
  for(const auto& subview : excludeFromSubviews)
    d->excludeFromSubviews.insert(subview);
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
void Layer::setExcludeFromSubviews(std::initializer_list<tp_utils::StringID> excludeFromSubviews)
{
  d->excludeFromSubviews = excludeFromSubviews;
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
const std::vector<tp_utils::StringID>& Layer::onlyInSubviews() const
{
  return d->onlyInSubviews;
}

//##################################################################################################
void Layer::setOnlyInSubviews(const std::unordered_set<tp_utils::StringID>& onlyInSubviews)
{
  d->onlyInSubviews.clear();
  d->onlyInSubviews.reserve(onlyInSubviews.size());
  for(const auto& subview : onlyInSubviews)
    d->onlyInSubviews.push_back(subview);
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
void Layer::setOnlyInSubviews(const std::vector<tp_utils::StringID>& onlyInSubviews)
{
  d->onlyInSubviews = onlyInSubviews;
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
void Layer::setOnlyInSubviews(std::initializer_list<tp_utils::StringID> onlyInSubviews)
{
  d->onlyInSubviews = onlyInSubviews;
  d->inheritSubviews = false;
  d->propagateSubviews();
}

//##################################################################################################
void Layer::addChildLayer(Layer* layer)
{
  insertChildLayer(d->layers.size(), layer);
}

//##################################################################################################
void Layer::insertChildLayer(size_t i, Layer *layer)
{
  if(layer->map())
  {
    tpWarning() << "Error! Map::insertLayer inserting a layer that is already in a map!";
    tp_utils::printStackTrace();
    return;
  }

  d->layers.insert(d->layers.begin()+int(i), layer);
  layer->setMap(map(), this);
  d->propagateSubviews();
  update();
}

//##################################################################################################
void Layer::removeChildLayer(Layer* layer)
{
  tpRemoveOne(d->layers, layer);
  layer->clearMap();
}

//##################################################################################################
void Layer::clearChildLayers()
{
  while(!d->layers.empty())
    delete d->layers.at(d->layers.size()-1);
}

//##################################################################################################
const std::vector<Layer*>& Layer::childLayers() const
{
  return d->layers;
}

//##################################################################################################
std::vector<Layer*>& Layer::childLayers()
{
  return d->layers;
}

//##################################################################################################
void Layer::render(RenderInfo& renderInfo)
{
  TP_FUNCTION_TIME("Layer::render");

  if(renderInfo.isPickingRender())
  {
    for(auto l : d->layers)
      if(l->visibileToCurrentSubview() && !l->excludeFromPicking())
        l->render(renderInfo);
  }
  else
  {
    for(auto l : d->layers)
      if(l->visibileToCurrentSubview())
        l->render(renderInfo);
  }
}

//##################################################################################################
void Layer::invalidateBuffers()
{
  invalidateBuffersCallbacks();

  for(auto i : d->layers)
    i->invalidateBuffers();
}

//##################################################################################################
bool Layer::mouseEvent(const MouseEvent& event)
{
  if(event.type == MouseEventType::Release)
  {
    for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    {
      Layer* layer = (*(--l));
      if(auto i = layer->m_hasMouseFocusFor.find(event.button); i!=layer->m_hasMouseFocusFor.end())
      {
        layer->m_hasMouseFocusFor.erase(i);
        if(layer->mouseEvent(event))
          return true;
      }
    }
  }

  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
  {
    Layer* layer = (*(--l));
    if(layer->mouseEvent(event))
    {
      if(event.type == MouseEventType::Press)
        layer->m_hasMouseFocusFor.insert(event.button);
      return true;
    }
  }

  return false;
}

//##################################################################################################
bool Layer::keyEvent(const KeyEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->keyEvent(event))
      return true;
  return false;
}

//################################################################################################
bool Layer::dragDropEvent(const DragDropEvent& event)
{
  for(size_t i=d->layers.size()-1; i<d->layers.size(); i--)
    if(d->layers.at(i)->dragDropEvent(event))
      return true;
  return false;
}

//##################################################################################################
bool Layer::textEditingEvent(const TextEditingEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->textEditingEvent(event))
      return true;
  return false;
}

//##################################################################################################
bool Layer::textInputEvent(const TextInputEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->textInputEvent(event))
      return true;
  return false;
}

//##################################################################################################
void Layer::animate(double timestampMS)
{
  for(auto l : d->layers)
    l->animate(timestampMS);
}

//##################################################################################################
void Layer::lightsChanged(LightingModelChanged lightingModelChanged)
{
  for(auto l : d->layers)
    l->lightsChanged(lightingModelChanged);
}

//##################################################################################################
void Layer::mapResized(int w, int h)
{
  for(auto layer : d->layers)
    layer->mapResized(w, h);
}

//##################################################################################################
void Layer::update(RenderFromStage renderFromStage)
{
  if(d->map)
  {
    if(!d->onlyInSubviews.empty())
      d->map->update(renderFromStage, d->onlyInSubviews);

    else
    {
      std::vector<tp_utils::StringID> subviews = d->map->allSubviewNames();

      for(const auto& excluded : d->excludeFromSubviews)
        tpRemoveOne(subviews, excluded);

      d->map->update(renderFromStage, subviews);
    }
  }
}

//##################################################################################################
void Layer::update(RenderFromStage renderFromStage, const std::vector<tp_utils::StringID>& subviews)
{
  d->map->update(renderFromStage, subviews);
}

//##################################################################################################
void Layer::callAsync(const std::function<void()>& callback)
{
  if(d->map)
  {
    std::weak_ptr<int> alive = d->alive;
    d->map->callAsync([=]
    {
      auto lock = alive.lock();
      if(lock)
      {
        callback();
      }
    });
  }
}

//##################################################################################################
void Layer::addedToMap()
{

}

//##################################################################################################
void Layer::childLayerDestroyed(Layer* layer)
{
  tpRemoveOne(d->layers, layer);
  update();
}

//##################################################################################################
void Layer::setMap(Map* map, Layer* parent)
{
  d->map = map;
  d->parent = parent;
  for(auto layer : d->layers)
    layer->setMap(map, this);
  addedToMap();
}

//##################################################################################################
void Layer::clearMap()
{
  d->map = nullptr;
  d->parent = nullptr;
}

//##################################################################################################
void Layer::addPointer(LayerPointer* layerPointer)
{
  d->layerPointers.push_back(layerPointer);
}

//##################################################################################################
void Layer::removePointer(LayerPointer* layerPointer)
{
  tpRemoveOne(d->layerPointers, layerPointer);
}

}
