#include "tp_maps/Map.h"
#include "tp_maps/Errors.h"
#include "tp_maps/Shader.h"
#include "tp_maps/Layer.h"
#include "tp_maps/layers/PostGammaLayer.h"
#include "tp_maps/controllers/FlatController.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"
#include "tp_maps/DragDropEvent.h"
#include "tp_maps/FontRenderer.h"
#include "tp_maps/SwapRowOrder.h"
#include "tp_maps/RenderModeManager.h"
#include "tp_maps/event_handlers/MouseEventHandler.h"
#include "tp_maps/subsystems/open_gl/OpenGLBuffers.h"

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/Ray.h"
#include "tp_math_utils/Intersection.h"

#include "tp_utils/StringID.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/StackTrace.h"
#include "tp_utils/Profiler.h" // IWYU pragma: keep
#include "tp_utils/Progress.h" // IWYU pragma: keep


#include "glm/glm.hpp" // IWYU pragma: keep
#include "glm/gtx/norm.hpp" // IWYU pragma: keep

#include <numeric>
#include <algorithm>

// Note: GL

#ifdef TP_EMSCRIPTEN
#include "tp_maps/shaders/PostBlitShader.h"
#define TP_BLIT_WITH_SHADER
#endif

#ifdef TP_MAPS_DEBUG
#  define DEBUG_printOpenGLError(A) if(auto e=glGetError(); e!=GL_NO_ERROR)Errors::printOpenGLError((A), e)
#else
#  define DEBUG_printOpenGLError(A) do{}while(false)
#endif

#if defined(TP_ENABLE_PROFILING) and not defined(SCOPED_DEBUG)
#define SCOPED_DEBUG
#endif

#if defined(TP_MAPS_DEBUG) and not defined(SCOPED_DEBUG)
#define SCOPED_DEBUG
#endif

#if defined(TP_ENABLE_FUNCTION_TIME) and not defined(SCOPED_DEBUG)
#define SCOPED_DEBUG
#endif

namespace tp_maps
{

namespace
{

#ifdef SCOPED_DEBUG
//##################################################################################################
class ScopedDebug_lt
{
  TP_NONCOPYABLE(ScopedDebug_lt);

#ifdef TP_ENABLE_PROFILING
  tp_utils::Profiler* m_profiler;
#endif

  const std::string m_name;

#ifdef TP_ENABLE_FUNCTION_TIME
  const char* m_file;
  const int m_line;
  tp_utils::FunctionTimer m_functionTimer{m_file, m_line, m_name.c_str()};
#endif
  const TPPixel m_color;

public:
  //################################################################################################
  ScopedDebug_lt(
    #ifdef TP_ENABLE_PROFILING
      tp_utils::Profiler* profiler,
    #endif
      const char* file,
      int line,
      const std::string& name,
      TPPixel color):
  #ifdef TP_ENABLE_PROFILING
    m_profiler(profiler),
  #endif
    m_name(name),
  #ifdef TP_ENABLE_FUNCTION_TIME
    m_file(file),
    m_line(line),
  #endif
    m_color(color)
  {
    TP_UNUSED(m_color);

#ifndef TP_ENABLE_FUNCTION_TIME
    TP_UNUSED(file);
    TP_UNUSED(line);
#endif
#ifdef TP_ENABLE_PROFILING
    if(m_profiler)
      m_profiler->rangePush(m_name, m_color);
#endif

#ifdef TP_MAPS_DEBUG
    if(auto e=glGetError(); e!=GL_NO_ERROR)
      Errors::printOpenGLError(m_name + " start", e);
#endif
  }

  //################################################################################################
  ~ScopedDebug_lt()
  {
#ifdef TP_ENABLE_PROFILING
    if(m_profiler)
    {
      glFinish();
      m_profiler->rangePop();
    }
#endif

#ifdef TP_MAPS_DEBUG
    if(auto e=glGetError(); e!=GL_NO_ERROR)
      Errors::printOpenGLError(m_name + " end", e);
#endif
  }
};

#ifdef TP_ENABLE_PROFILING
#define DEBUG_scopedDebug(A, C) ScopedDebug_lt TP_CONCAT(scopedDebug, __LINE__)(d->profiler.get(), __FILE__, __LINE__, A, C); TP_UNUSED(TP_CONCAT(scopedDebug, __LINE__))
#else
#define DEBUG_scopedDebug(A, C) ScopedDebug_lt TP_CONCAT(scopedDebug, __LINE__)(__FILE__, __LINE__, A, C); TP_UNUSED(TP_CONCAT(scopedDebug, __LINE__))
#endif

#else
#define DEBUG_scopedDebug(A, C) do{}while(false)
#endif

//##################################################################################################
struct CustomPassCallbacks_lt
{
  std::function<void(RenderInfo&)> start;
  std::function<void(RenderInfo&)> end;
};

//##################################################################################################
struct EventHandler_lt
{
  size_t eventHandlerId{0};
  int priority{0};
  EventHandlerCallbacks callbacks;

  std::unordered_set<Button> m_hasMouseFocusFor;
  std::unordered_set<int32_t> m_hasKeyFocusFor;
};
}

//##################################################################################################
struct Map::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Map::Private");
  TP_NONCOPYABLE(Private);

  Map* q;
  Errors errors;
  OpenGLBuffers buffers;

  Controller* controller{nullptr};
  std::vector<Layer*> layers;
  std::unordered_map<tp_utils::StringID, Shader*> shaders;
  std::vector<FontRenderer*> fontRenderers;

  MouseEventHandler* mouseEventHandler{new MouseEventHandler(q)};

  size_t nextEventHandlerId{0};
  std::vector<std::shared_ptr<EventHandler_lt>> eventHandlers;

  std::vector<RenderPass> renderPasses;

  std::vector<RenderPass> computedRenderPasses;

  RenderFromStage renderFromStage{RenderFromStage::Full};

  //  // Callback that get called to prepare and cleanup custom render passes.
  //  CustomPassCallbacks_lt customCallbacks[int(RenderPass::CustomEnd) - int(RenderPass::Custom1)];

  RenderInfo renderInfo;

  size_t width{1};
  size_t height{1};
  glm::vec4 backgroundColor{0.0f, 0.0f, 0.0f, 1.0f};
  GLboolean writeAlpha{GL_FALSE};

  ShaderProfile shaderProfile{TP_DEFAULT_PROFILE};
  bool visible{true};

  // The first is multisampled the second and third are not.
  // We don't want to multisample multiple times it just makes the result blury. So what we do here
  // is have 3 buffers, the first has the 3D geometry drawn to it and is multisampled after this the
  // render pipeline toggles between the second and third for the remaining post processing steps.
  std::unordered_map<tp_utils::StringID, std::unique_ptr<OpenGLFBO>> intermediateFBOs;

  OpenGLFBO* currentReadFBO{intermediateFBO(defaultSID())};
  OpenGLFBO* currentDrawFBO{intermediateFBO(defaultSID())};

  std::vector<tp_math_utils::Light> lights;
  std::vector<OpenGLFBO> lightBuffers;
  size_t lightTextureSize{1024};

  tp_utils::ElapsedTimer renderTimer;

  HDR hdr{HDR::No};
  ExtendedFBO extendedFBO{ExtendedFBO::No};

  OpenGLFBO pickingBuffer;
  OpenGLFBO renderToImageBuffer;

#ifdef TP_BLIT_WITH_SHADER
  FullScreenShader::Object* rectangleObject{nullptr};
#endif

  double previousAnimateTimestamp{0.0};
  double timeSincePreviousAnimate{0.0};

  bool initialized{false};
  bool preDeleteCalled{false};

  RenderModeManager* renderModeManager{nullptr};

#ifdef TP_ENABLE_PROFILING
  std::shared_ptr<tp_utils::Profiler> profiler{nullptr};
#endif

  //################################################################################################
  Private(Map* q_):
    q(q_),
    errors(q),
    buffers(q)
  {
    {
      tp_math_utils::Light light;
      light.type = tp_math_utils::LightType::Spot;

      light.setPosition({1.52f, -1.52f, 5.4f});
      light.setDirection({-0.276, 0.276, -0.92});

      light.ambient  = {0.2f, 0.2f, 0.2f};
      light.diffuse  = {0.3f, 0.3f, 0.3f};
      light.specular = {1.0f, 1.0f, 1.0f};

      lights.push_back(light);
    }

    {
      tp_math_utils::Light light;
      light.type = tp_math_utils::LightType::Directional;

      light.setPosition({-5.52f, -5.52f, 18.4f});
      light.setDirection({0.276, 0.276, -0.92});

      light.ambient  = {0.2f, 0.2f, 0.2f};
      light.diffuse  = {0.3f, 0.3f, 0.3f};
      light.specular = {1.0f, 1.0f, 1.0f};

      lights.push_back(light);
    }
    
    renderInfo.map = q;
  }

  //################################################################################################
  void deleteShaders()
  {
    if(shaders.empty())
      return;

    q->makeCurrent();

    for(const auto& i : shaders)
      delete i.second;

    shaders.clear();

#ifdef TP_BLIT_WITH_SHADER
    delete rectangleObject;
    rectangleObject = nullptr;
#endif
  }

  //################################################################################################
  OpenGLFBO* intermediateFBO(const tp_utils::StringID& name)
  {
    auto& intermediateFBO = intermediateFBOs[name];
    if(!intermediateFBO)
      intermediateFBO = std::make_unique<OpenGLFBO>();
    return intermediateFBO.get();
  }

  //################################################################################################
  void render()
  {
    controller->updateMatrices();

    auto render = [&](auto test)
    {
      for(auto l : layers)
      {
        if(test(l))
        {
          try
          {
            l->render(renderInfo);
          }
          catch (const std::exception& ex)
          {
            tpWarning() << "Exception caught in Map::Private::render!";
            tpWarning() << "Exception: " << ex.what();
          }
          catch (...)
          {
            tpWarning() << "Exception caught in Map::Private::render!";
          }
        }
      }
    };

    if(renderInfo.isPickingRender())
      render([](auto l){return l->visible() && !l->excludeFromPicking();});
    else
      render([](auto l){return l->visible();});
  }

  //################################################################################################
  void setRenderPassesInternal(const std::vector<RenderPass>& renderPasses)
  {
    for(const auto& renderPass : this->renderPasses)
      delete renderPass.postLayer;

    this->renderPasses = renderPasses;
    for(const auto& renderPass : this->renderPasses)
      if(renderPass.postLayer)
        q->insertLayer(0, renderPass.postLayer);
  }

  //################################################################################################
  bool hasRenderPass(RenderPass::RenderPassType renderPassType)
  {
    for(const auto& renderPass : renderPasses)
      if(renderPass.type == renderPassType)
        return true;
    return false;
  }

  //################################################################################################
  void callMapResized()
  {
    controller->mapResized(int(width), int(height));

    for(auto layer : layers)
      layer->mapResized(int(width), int(height));
  }
};

//##################################################################################################
Map::Map(bool enableDepthBuffer):
  d(new Private(this))
{
  TP_UNUSED(enableDepthBuffer);

  d->renderModeManager = new RenderModeManager(this);

  d->controller = new FlatController(this);

  d->setRenderPassesInternal({
                               tp_maps::RenderPass::LightFBOs     ,
                               tp_maps::RenderPass::PrepareDrawFBO,
                               tp_maps::RenderPass::Background    ,
                               tp_maps::RenderPass::Normal        ,
                               tp_maps::RenderPass::Transparency  ,
                               tp_maps::RenderPass::FinishDrawFBO ,
                               new tp_maps::PostGammaLayer()      ,
                               tp_maps::RenderPass::Text          ,
                               tp_maps::RenderPass::GUI3D          ,
                               tp_maps::RenderPass::GUI
                             });
}

//##################################################################################################
Map::~Map()
{
  if(!d->preDeleteCalled)
    tpWarning() << "Error! Map subclasses must call preDelete in their destructor!";
  delete d;
}

//##################################################################################################
void Map::preDelete()
{
  d->deleteShaders();

  delete d->renderModeManager;
  d->renderModeManager = nullptr;

  delete d->controller;
  d->controller=nullptr;

  clearLayers();

  while(!d->fontRenderers.empty())
    delete tpTakeFirst(d->fontRenderers);

  delete d->mouseEventHandler;
  d->mouseEventHandler = nullptr;

  for(auto& i : d->intermediateFBOs)
    d->buffers.deleteBuffer(*i.second.get());

  d->buffers.deleteBuffer(d->pickingBuffer);
  d->buffers.deleteBuffer(d->renderToImageBuffer);

  for(auto& lightBuffer : d->lightBuffers)
    d->buffers.deleteBuffer(lightBuffer);

#ifdef TP_BLIT_WITH_SHADER
  delete d->rectangleObject;
  d->rectangleObject = nullptr;
#endif

  d->preDeleteCalled = true;
}

//##################################################################################################
void Map::setShaderProfile(ShaderProfile shaderProfile)
{
  d->shaderProfile = shaderProfile;
}

//##################################################################################################
void Map::setVisible(bool visible)
{
  d->visible = visible;
}

#ifdef TP_ENABLE_PROFILING
//##################################################################################################
void Map::setProfiler(const std::shared_ptr<tp_utils::Profiler>& profiler)
{
  d->profiler = profiler;

  if(d->profiler)
  {
    d->profiler->clearSummaryGenerators();
    d->profiler->addSummaryGenerator([](const auto& profiler, auto& summaries)
    {
      int64_t min{0};
      int64_t max{0};
      int64_t sum{0};
      size_t count{0};
      profiler.viewProgressEvents([&](const auto& progressEvents)
      {
        for(auto& event : progressEvents)
        {
          if(event.name == "Frame")
          {
            int64_t duration = event.end - event.start;

            if(duration<min || count==0)
              min = duration;

            if(duration>max)
              max = duration;

            sum+=duration;
            count++;
          }
        }
      });

      double average=(count>0)?double(sum) / double(count):0.0;
      summaries.emplace_back("Minimum frame duration", std::to_string(min)+"ms");
      summaries.emplace_back("Maximum frame duration", std::to_string(max)+"ms");
      summaries.emplace_back("Average frame duration", std::to_string(average)+"ms");
      summaries.emplace_back("Number of frames", std::to_string(count));
    });
  }
}

//##################################################################################################
const std::shared_ptr<tp_utils::Profiler>& Map::profiler() const
{
  return d->profiler;
}
#endif

//##################################################################################################
ShaderProfile Map::shaderProfile() const
{
  return d->shaderProfile;
}

//##################################################################################################
bool Map::visible() const
{
  return d->visible;
}

//##################################################################################################
bool Map::initialized() const
{
  return d->initialized;
}

//##################################################################################################
RenderModeManager& Map::renderModeManger() const
{
  return *d->renderModeManager;
}

//##################################################################################################
const OpenGLBuffers& Map::buffers() const
{
  return d->buffers;
}

//##################################################################################################
RenderInfo& Map::renderInfo()
{
  return d->renderInfo;
}

//##################################################################################################
void Map::animate(double timestampMS)
{
  d->timeSincePreviousAnimate = timestampMS - d->previousAnimateTimestamp;
  d->previousAnimateTimestamp = timestampMS;

  d->controller->animate(timestampMS);

  for(auto l : d->layers)
    l->animate(timestampMS);

  animateCallbacks(timestampMS);
}

//##################################################################################################
double Map::timeSincePreviousAnimate() const
{
  return d->timeSincePreviousAnimate;
}

namespace
{
//##################################################################################################
Map*& inPaintState()
{
  static Map* inPaint{nullptr};
  return inPaint;
}
}

//##################################################################################################
Map* Map::inPaint() const
{
  return inPaintState();
}

//##################################################################################################
void Map::setInPaint(bool inPaint)
{
  Map*& s=inPaintState();
  if(inPaint)
  {
    assert(s!=this); // This has already set inPaint.
    assert(!s);      // Something else has already set inPaint.
    s=this;
  }
  else
  {
    assert(s==this); // This has NOT set inPaint.
    s=nullptr;
  }
}

//##################################################################################################
void Map::invalidateBuffers()
{
  d->initialized = false;

  for(auto& i : d->shaders)
  {
    i.second->invalidate();
    delete i.second;
  }
  d->shaders.clear();

  d->intermediateFBOs.clear();

  d->buffers.invalidateBuffer(d->pickingBuffer);
  d->buffers.invalidateBuffer(d->renderToImageBuffer);

  for(auto& lightTexture : d->lightBuffers)
    d->buffers.invalidateBuffer(lightTexture);
  d->lightBuffers.clear();

  for(auto i : d->layers)
    i->invalidateBuffers();

  for(auto i : d->fontRenderers)
    i->invalidateBuffers();

#ifdef TP_BLIT_WITH_SHADER
  delete d->rectangleObject;
  d->rectangleObject = nullptr;
#endif

  invalidateBuffersCallbacks();
}

//##################################################################################################
Controller* Map::controller()
{
  return d->controller;
}

//##################################################################################################
void Map::setBackgroundColor(const glm::vec3& color)
{
  d->backgroundColor = {color.x, color.y, color.z, 1.0f};
  update();
}

//##################################################################################################
void Map::setBackgroundColor(const glm::vec4& color)
{
  d->backgroundColor = color;
  update();
}

//##################################################################################################
glm::vec3 Map::backgroundColor() const
{
  return d->backgroundColor;
}

//##################################################################################################
void Map::setWriteAlpha(GLboolean writeAlpha)
{
  d->writeAlpha = writeAlpha;
}

//##################################################################################################
GLboolean Map::writeAlpha() const
{
  return d->writeAlpha;
}

//##################################################################################################
void Map::setRenderPasses(const std::vector<RenderPass>& renderPasses)
{
  if(!d->intermediateFBOs.empty() && this->initialized())
  {
    makeCurrent();
    for(auto& i : d->intermediateFBOs)
      d->buffers.deleteBuffer(*i.second.get());
    d->intermediateFBOs.clear();
  }

  d->setRenderPassesInternal(renderPasses);
}

//##################################################################################################
void Map::setLights(const std::vector<tp_math_utils::Light>& lights)
{
  if(inPaint() && d->renderInfo.pass != RenderPass::PreRender)
  {
    tpWarning() << "Error lights set while in render. This is only valid in the PreRender pass!";
    tp_utils::printStackTrace();
    return;
  }

  LightingModelChanged lightingModelChanged=LightingModelChanged::No;

  if(d->lights.size() != lights.size())
    lightingModelChanged=LightingModelChanged::Yes;
  else
  {
    for(size_t i=0; i<lights.size(); i++)
    {
      if(d->lights.at(i).type != lights.at(i).type)
      {
        lightingModelChanged=LightingModelChanged::Yes;
        break;
      }

      if(std::fabs(glm::distance2(d->lights.at(i).offsetScale, lights.at(i).offsetScale)) > 0.00001f)
      {
        lightingModelChanged=LightingModelChanged::Yes;
        break;
      }
    }
  }

  d->lights = lights;

  if(lightingModelChanged==LightingModelChanged::Yes)
    d->deleteShaders();

  for(auto l : d->layers)
    l->lightsChanged(lightingModelChanged);

  update();
}

//##################################################################################################
const std::vector<tp_math_utils::Light>& Map::lights() const
{
  return d->lights;
}

//##################################################################################################
void Map::addLayer(Layer* layer)
{
  insertLayer(d->layers.size(), layer);
}

//##################################################################################################
void Map::insertLayer(size_t i, Layer* layer)
{
  if(layer->map())
  {
    tpWarning() << "Error! Map::insertLayer inserting a layer that is already in a map!";
    tp_utils::printStackTrace();
    return;
  }

  d->layers.insert(d->layers.begin()+int(i), layer);
  layer->setMap(this, nullptr);

  layerInserted(i, layer);

  update();
}

//##################################################################################################
void Map::removeLayer(Layer* layer)
{
  tpRemoveOne(d->layers, layer);
  layer->clearMap();
}

//##################################################################################################
void Map::clearLayers()
{
  while(!d->layers.empty())
    delete d->layers.at(d->layers.size()-1);
}

//##################################################################################################
const std::vector<Layer*>& Map::layers() const
{
  return d->layers;
}

//##################################################################################################
void Map::resetController()
{
  d->controller = new FlatController(this);
}

//##################################################################################################
void Map::setMaxLightTextureSize(size_t lightTextureSize)
{
  d->lightTextureSize = lightTextureSize;
}

//##################################################################################################
size_t Map::maxLightTextureSize() const
{
  return d->lightTextureSize;
}

//##################################################################################################
void Map::setMaxSamples(size_t maxSamples)
{
  d->buffers.setMaxSamples(maxSamples);
}

//##################################################################################################
size_t Map::maxSamples() const
{
  return d->buffers.maxSamples();
}

//##################################################################################################
void Map::setHDR(HDR hdr)
{
  if(d->hdr != hdr)
  {
    d->hdr = hdr;
    d->deleteShaders();
    update();
  }
}

//##################################################################################################
HDR Map::hdr() const
{
  return d->hdr;
}

//##################################################################################################
void Map::setExtendedFBO(ExtendedFBO extendedFBO)
{
  if(d->extendedFBO != extendedFBO)
  {
    d->extendedFBO = extendedFBO;
    d->deleteShaders();
    update();
  }
}

//##################################################################################################
ExtendedFBO Map::extendedFBO() const
{
  return d->extendedFBO;
}

//##################################################################################################
std::vector<Layer*>& Map::layers()
{
  return d->layers;
}

//##################################################################################################
void Map::project(const glm::vec3& scenePoint, glm::vec2& screenPoint, const glm::mat4& matrix)
{
  glm::vec4 v = matrix * glm::vec4(scenePoint, 1.0f);

  screenPoint = v;
  if(std::fabs(v.w)>0.00001f)
    screenPoint /= v.w;
  screenPoint = (screenPoint+1.0f)/2.0f;
  screenPoint.x = screenPoint.x * float(d->width);
  screenPoint.y = float(d->height) - (screenPoint.y * float(d->height));
}

//##################################################################################################
void Map::project(const glm::vec3& scenePoint, glm::vec3& screenPoint, const glm::mat4& matrix)
{
  glm::vec4 v = matrix * glm::vec4(scenePoint, 1.0f);

  screenPoint = v;
  if(std::fabs(v.w)>0.00001f)
    screenPoint /= v.w;

  screenPoint.x = (screenPoint.x+1.0f)/2.0f;
  screenPoint.y = (screenPoint.y+1.0f)/2.0f;

  screenPoint.x = screenPoint.x * float(d->width);
  screenPoint.y = float(d->height) - (screenPoint.y * float(d->height));
}

//##################################################################################################
void Map::projectGL(const glm::vec3& scenePoint, glm::vec2& screenPoint, const glm::mat4& matrix)
{
  glm::vec4 v = matrix * glm::vec4(scenePoint, 1.0f);
  v /= v.w;

  v = (v+1.0f)/2.0f;

  screenPoint.x = v.x * float(d->width);
  screenPoint.y = v.y * float(d->height);
}

//##################################################################################################
bool Map::unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane)
{
  return unProject(screenPoint, scenePoint, plane, d->controller->matrix(defaultSID()));
}

//##################################################################################################
bool Map::unProject(const glm::dvec2& screenPoint, glm::dvec3& scenePoint, const tp_math_utils::Plane& plane)
{
  return unProject(screenPoint, scenePoint, plane, d->controller->matrices(defaultSID()).dvp);
}

//##################################################################################################
bool Map::unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane, const glm::mat4& matrix)
{
  glm::mat4 inverse = glm::inverse(matrix);

  std::array<glm::vec4, 2> screenPoints{};
  std::array<glm::vec3, 2> scenePoints{};

  screenPoints[0] = glm::vec4(screenPoint, 0.0f, 1.0f);
  screenPoints[1] = glm::vec4(screenPoint, 1.0f, 1.0f);

  for(size_t i=0; i<2; i++)
  {
    glm::vec4& tmp = screenPoints.at(i);
    tmp.x = tmp.x / float(d->width);
    tmp.y = (float(d->height) - tmp.y) / float(d->height);
    tmp = tmp * 2.0f - 1.0f;

    glm::vec4 obj = inverse * tmp;
    obj /= obj.w;
    scenePoints.at(i) = glm::vec3(obj);
  }

  return tp_math_utils::rayPlaneIntersection(tp_math_utils::Ray(scenePoints[0], scenePoints[1]), plane, scenePoint);
}

//##################################################################################################
bool Map::unProject(const glm::dvec2& screenPoint, glm::dvec3& scenePoint, const tp_math_utils::Plane& plane, const glm::dmat4& matrix)
{
  glm::mat4 inverse = glm::inverse(matrix);

  std::array<glm::dvec4, 2> screenPoints{};
  std::array<glm::dvec3, 2> scenePoints{};

  screenPoints[0] = glm::dvec4(screenPoint, 0.0, 1.0);
  screenPoints[1] = glm::dvec4(screenPoint, 1.0, 1.0);

  for(size_t i=0; i<2; i++)
  {
    glm::dvec4& tmp = screenPoints.at(i);
    tmp.x = tmp.x / double(d->width);
    tmp.y = (double(d->height) - tmp.y) / double(d->height);
    tmp = tmp * 2.0 - 1.0;

    glm::dvec4 obj = inverse * tmp;
    obj /= obj.w;
    scenePoints.at(i) = glm::dvec3(obj);
  }

  return tp_math_utils::rayPlaneIntersection(tp_math_utils::Ray(scenePoints[0], scenePoints[1]), plane, scenePoint);
}

//##################################################################################################
glm::vec3 Map::unProject(const glm::vec3& screenPoint)
{
  return unProject(screenPoint, d->controller->matrix(defaultSID()));
}

//##################################################################################################
glm::vec3 Map::unProject(const glm::vec3& screenPoint, const glm::mat4& matrix)
{
  glm::mat4 inverse = glm::inverse(matrix);

  glm::vec4 tmp{screenPoint, 1.0f};
  tmp.x = tmp.x / float(d->width);
  tmp.y = (float(d->height) - tmp.y) / float(d->height);
  tmp.x = tmp.x * 2.0f - 1.0f;
  tmp.y = tmp.y * 2.0f - 1.0f;
  glm::vec4 obj = inverse * tmp;

  return obj / obj.w;
}

namespace
{
//##################################################################################################
std::vector<glm::ivec2> generateTestOrder(int size)
{
  std::vector<glm::ivec2> testOrder;
  glm::ivec2 current(size/2, size/2);

  testOrder.push_back(current);
  testOrder.reserve(size*size);

  int direction=0;

  for(int i=1; i<size; i++)
  {
    for(int p=0; p<2; p++)
    {
      glm::ivec2 vector(0, 0);

      switch(direction)
      {
        case 0: vector.x =  1; break;
        case 1: vector.y = -1; break;
        case 2: vector.x = -1; break;
        case 3: vector.y =  1; break;
      }

      for(int s=0; s<i; s++)
      {
        current+=vector;
        testOrder.push_back(current);
      }

      direction = (direction+1) % 4;
    }
  }

  glm::ivec2 vector(0, 0);
  switch(direction)
  {
    case 0: vector.x =  1; break;
    case 1: vector.y = -1; break;
    case 2: vector.x = -1; break;
    case 3: vector.y =  1; break;
  }

  for(int s=1; s<size; s++)
  {
    current+=vector;
    testOrder.push_back(current);
  }

  return testOrder;
}
}

//##################################################################################################
PickingResult* Map::performPicking(const tp_utils::StringID& pickingType, const glm::ivec2& pos)
{
  const int pickingSize=9;
  const int left=pickingSize/2;

  if(d->width<pickingSize || d->height<pickingSize)
    return nullptr;

  makeCurrent();
  setInPaint(true);
  TP_CLEANUP([&]{setInPaint(false);});

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  //------------------------------------------------------------------------------------------------
  // Configure the frame buffer that the picking values will be rendered to.
  if(!d->buffers.prepareBuffer("renderToImage",
                               d->renderToImageBuffer,
                               d->width,
                               d->height,
                               CreateColorBuffer::Yes,
                               Multisample::No,
                               HDR::No,
                               ExtendedFBO::No,
                               true))
    return nullptr;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //------------------------------------------------------------------------------------------------
  // Execute a picking render pass.

  // 3D Geometry
  {
    d->renderInfo.resetPicking();
    d->renderInfo.pass = RenderPass::Picking;
    d->renderInfo.hdr = HDR::No;
    d->renderInfo.extendedFBO = ExtendedFBO::No;
    d->renderInfo.pickingType = pickingType;
    d->renderInfo.pos = pos;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(true);

    d->render();
  }

  // 3D GUI Geometry
  if(d->hasRenderPass(RenderPass::GUI3D))
  {
    glClear(GL_DEPTH_BUFFER_BIT);
    d->renderInfo.pass = RenderPass::PickingGUI3D;
    d->render();
  }

  //------------------------------------------------------------------------------------------------
  // Read the small patch from around the picking position and then free up the frame buffers.

  //The size of the area to perform picking in, must be an odd number
  int windowX = tpBound(0, pos.x-left, width()-(pickingSize+1));
  int windowY = tpBound(0, (int(d->height)-pos.y)-left, int(height()-(pickingSize+1)));
  std::vector<unsigned char> pixels(pickingSize*pickingSize*4);

  switch(d->shaderProfile)
  {
    case ShaderProfile::GLSL_100_ES:
    break;

    default:
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    break;
  }

  glReadPixels(windowX, windowY, pickingSize, pickingSize, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);

  //------------------------------------------------------------------------------------------------
  // Iterate over the 9x9 patch of picking data and find the most appropriate result. We favor
  // results near the center of the the patch.

  static const std::vector<glm::ivec2> testOrder(generateTestOrder(pickingSize));
  for(const auto& point : testOrder)
  {
    unsigned char* p = pixels.data() + (((point.y*9)+point.x)*4);

    uint32_t value = RenderInfo::pickingIDFromColor(p[0], p[1], p[2]);

    if(value>0)
    {
      uint32_t count=0;
      for(auto& pickingDetails : d->renderInfo.pickingDetails)
      {
        uint32_t index = value-count;
        count+=pickingDetails.count;
        if(value<count && index<pickingDetails.count)
        {
          pickingDetails.index += size_t(index);
          return (pickingDetails.callback)?
                pickingDetails.callback(PickingResult(pickingType, pickingDetails, d->renderInfo, nullptr)):
                nullptr;
        }
      }
    }
  }

  return nullptr;
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, tp_image_utils::ColorMap& image, bool swapY)
{
  image.setSize(width, height);
  return renderToImage(width, height, image.data(), swapY);
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, TPPixel* pixels, bool swapY)
{
  return renderToImage(width, height, HDR::No, [&]
  {
    // Read the texture that we just generated
    glReadPixels(0, 0, int(width), int(height), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    DEBUG_printOpenGLError("renderToImage C3");

    if(swapY)
      swapRowOrder(width, height, pixels);
  });
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, tp_image_utils::ColorMapF& image, bool swapY)
{
  image.setSize(width, height);

  return renderToImage(width, height, HDR::Yes, [&]
  {
    // Read the texture that we just generated
    glm::vec4* pixels = image.data();
    glReadPixels(0, 0, int(width), int(height), GL_RGBA, GL_FLOAT, pixels);
    DEBUG_printOpenGLError("renderToImage C3");

    if(swapY)
      swapRowOrder(width, height, pixels);
  });
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, HDR hdr, const std::function<void()>& renderComplete)
{
  PRF_SCOPED_RANGE(d->profiler.get(), "Frame", {255,255,255});

  if(width<1 || height<1)
  {
    tpWarning() << "Error Map::renderToImage can't render to image smaller than 1 pixel.";
    return false;
  }

  auto originalWidth  = d->width;
  auto originalHeight = d->height;
  resizeGL(int(width), int(height));

  makeCurrent();
  setInPaint(true);
  TP_CLEANUP([&]{setInPaint(false);});

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  DEBUG_printOpenGLError("renderToImage A");

  // Configure the frame buffer that the image will be rendered to.
  if(!d->buffers.prepareBuffer("renderToImage",
                               d->renderToImageBuffer,
                               width,
                               height,
                               CreateColorBuffer::Yes,
                               Multisample::No,
                               hdr,
                               ExtendedFBO::No,
                               true))
  {
    tpWarning() << "Error Map::renderToImage failed to create render buffer.";
    return false;
  }

  DEBUG_printOpenGLError("renderToImage B");

  // Execute a render passes.
  paintGLNoMakeCurrent();
  DEBUG_printOpenGLError("renderToImage C1");

  // Swap the multisampled FBO into the non multisampled FBO.
  d->buffers.swapMultisampledBuffer(d->renderToImageBuffer);
  DEBUG_printOpenGLError("renderToImage C2");

  // Read the texture that we just generated
  glBindFramebuffer(GL_FRAMEBUFFER, d->renderToImageBuffer.frameBuffer);
  renderComplete();

  // Return to the original viewport settings
  d->width = originalWidth;
  d->height = originalHeight;
  resizeGL(int(originalWidth), int(originalHeight));
  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);
  glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));

  DEBUG_printOpenGLError("renderToImage D");

  return true;
}


//##################################################################################################
void Map::deleteTexture(GLuint id)
{
  TP_FUNCTION_TIME("Map::deleteTexture");
  if(id>0)
  {
    makeCurrent();
    glDeleteTextures(1, &id);
  }
}

//##################################################################################################
void Map::deleteShader(const tp_utils::StringID& name)
{
  auto i = d->shaders.find(name);
  if(i == d->shaders.end())
    return;

  makeCurrent();
  delete i->second;
  d->shaders.erase(i);

  update();
}

//##################################################################################################
const OpenGLFBO& Map::currentReadFBO()
{
  return *d->currentReadFBO;
}

//##################################################################################################
const OpenGLFBO& Map::currentDrawFBO()
{
  return *d->currentDrawFBO;
}

//##################################################################################################
const std::vector<OpenGLFBO>& Map::lightBuffers() const
{
  return d->lightBuffers;
}

//##################################################################################################
int Map::width() const
{
  return int(d->width);
}

//##################################################################################################
int Map::height() const
{
  return int(d->height);
}

//##################################################################################################
glm::vec2 Map::screenSize() const
{
  return {d->width, d->height};
}

//##################################################################################################
void Map::update(RenderFromStage renderFromStage)
{
  if(renderFromStage<d->renderFromStage)
    d->renderFromStage = renderFromStage;
}

//##################################################################################################
float Map::pixelScale() const
{
  return 1.0f;
}

//##################################################################################################
void Map::initializeGL()
{
#ifdef TP_WIN32
  static bool initGlew=[]
  {
    if(GLenum err = glewInit(); err!=GLEW_OK)
      tpWarning() << "Error initializing glew: " << err;
    return false;
  }();
  TP_UNUSED(initGlew);
#endif

  d->errors.initializeGL();
  d->buffers.initializeGL();

  //Invalidate old state before initializing new state
  invalidateBuffers();

  // Initialize GL
  // On some platforms the context isn't current, so fix that first
  makeCurrent();

  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DITHER);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

#ifdef TP_ENABLE_MULTISAMPLE
  glEnable(GL_MULTISAMPLE);
#endif
  tpWarning() << glGetString(GL_VERSION);

  d->initialized = true;
  d->renderFromStage = RenderFromStage::Full;

  d->callMapResized();
}

//##################################################################################################
void Map::paintGL()
{
  PRF_SCOPED_RANGE(d->profiler.get(), "Frame", {255,255,255});
  makeCurrent();
  setInPaint(true);
  TP_CLEANUP([&]{setInPaint(false);});
  paintGLNoMakeCurrent();
}

//##################################################################################################
void Map::paintGLNoMakeCurrent()
{
  DEBUG_printOpenGLError("paintGLNoMakeCurrent start");

  d->renderTimer.start();

  d->computedRenderPasses.clear();
  d->computedRenderPasses.reserve(d->renderPasses.size()*2);
  for(const auto& renderPass : d->renderPasses)
  {
    if(renderPass.type == RenderPass::Delegate)
      renderPass.postLayer->addRenderPasses(d->computedRenderPasses);
    else
      d->computedRenderPasses.push_back(renderPass);
  }
#if 0
  int ctr=0;
  for(const auto& rp : d->computedRenderPasses)
    tpWarning() << "Render pass: " << rp.describe() << " index: " << ctr++;
#endif
#ifdef TP_FBO_SUPPORTED
  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
#endif

  d->renderInfo.pass = RenderPass::PreRender;
  d->render();

  glDepthMask(true);
  glClearDepthf(1.0f);
  glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, d->backgroundColor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Skip the passes that don't need a full render.
  size_t rp = skipRenderPasses();

  d->renderFromStage = RenderFromStage::Reset;

#ifdef TP_FBO_SUPPORTED
  executeRenderPasses(rp, originalFrameBuffer);
#endif

  Errors::printOpenGLError("Map::paintGL");
}

//##################################################################################################
size_t Map::skipRenderPasses()
{
  size_t rp=0;
  if(d->renderFromStage != RenderFromStage::Full && d->renderFromStage != RenderFromStage::Reset)
  {
    for(; rp<d->computedRenderPasses.size(); rp++)
    {
      auto renderPass = d->computedRenderPasses.at(rp);

#ifdef TP_FBO_SUPPORTED
      switch(renderPass.type)
      {
        case RenderPass::PreRender: //--------------------------------------------------------------
        case RenderPass::LightFBOs: //--------------------------------------------------------------
        break;

        case RenderPass::PrepareDrawFBO: //---------------------------------------------------------
        {
          d->currentDrawFBO = d->intermediateFBO(defaultSID());
          d->currentReadFBO = d->intermediateFBO(defaultSID());
          break;
        }

        case RenderPass::SwapToFBO: //--------------------------------------------------------------
        {
          d->currentReadFBO = d->currentDrawFBO;
          d->currentDrawFBO = d->intermediateFBO(renderPass.name);
          break;
        }

        case RenderPass::SwapToFBONoClear: //-------------------------------------------------------
        {
          d->currentReadFBO = d->currentDrawFBO;
          d->currentDrawFBO = d->intermediateFBO(renderPass.name);
          break;
        }

        case RenderPass::BlitFromFBO: //------------------------------------------------------------
        case RenderPass::Background: //-------------------------------------------------------------
        case RenderPass::Normal: //-----------------------------------------------------------------
        case RenderPass::Transparency: //-----------------------------------------------------------
        break;

        case RenderPass::FinishDrawFBO: //----------------------------------------------------------
        {
          std::swap(d->currentDrawFBO, d->currentReadFBO);
          break;
        }

        case RenderPass::Text: //-------------------------------------------------------------------
        case RenderPass::GUI3D: //------------------------------------------------------------------
        case RenderPass::GUI: //--------------------------------------------------------------------
        case RenderPass::Picking: //----------------------------------------------------------------
        case RenderPass::PickingGUI3D: //-----------------------------------------------------------
        case RenderPass::Custom: //-----------------------------------------------------------------
        case RenderPass::Delegate: //---------------------------------------------------------------
        break;

        case RenderPass::Stage: //------------------------------------------------------------------
        {
          if(d->renderFromStage == RenderFromStage::Stage && renderPass.index == d->renderFromStage.index)
          {
            rp++;
            return rp;
          }

          break;
        }

      }
#endif
    }
  }

  return rp;
}

//##################################################################################################
void Map::executeRenderPasses(size_t rp, GLint& originalFrameBuffer)
{
  PRINT_FUNCTION_NAME("Map::executeRenderPasses");
  PRF_SCOPED_RANGE(d->profiler.get(), "executeRenderPasses", {255,255,255});

  for(; rp<d->computedRenderPasses.size(); rp++)
  {
    auto renderPass = d->computedRenderPasses.at(rp);
    try
    {
      if(renderPass.postLayer)
      {
        DEBUG_scopedDebug("prepareForRenderPass " + renderPass.getNameString(), TPPixel(90, 0, 150));
        renderPass.postLayer->prepareForRenderPass(renderPass);
      }

      d->renderInfo.pass = renderPass;
      switch(renderPass.type)
      {
        case RenderPass::PreRender: //--------------------------------------------------------------
        break;

        case RenderPass::LightFBOs: //--------------------------------------------------------------
        {
#ifdef TP_FBO_SUPPORTED
          DEBUG_scopedDebug("RenderPass::LightFBOs", TPPixel(255, 0, 0));
          d->renderInfo.hdr = HDR::No;
          d->renderInfo.extendedFBO = ExtendedFBO::No;

          while(d->lightBuffers.size() < d->lights.size())
            d->lightBuffers.emplace_back();

          while(d->lightBuffers.size() > d->lights.size())
          {
            auto buffer = tpTakeLast(d->lightBuffers);
            d->buffers.deleteBuffer(buffer);
          }
          DEBUG_printOpenGLError("RenderPass::LightFBOs delete buffers");

          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);
          glDepthMask(true);
          DEBUG_printOpenGLError("RenderPass::LightFBOs enable depth");

          for(size_t i=0; i<d->lightBuffers.size(); i++)
          {
            const auto& light = d->lights.at(i);
            auto& lightBuffer = d->lightBuffers.at(i);

            DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers (A)");
            if(!d->buffers.prepareBuffer(std::string( "lightBuffer_" ) + std::to_string(i),
                                         lightBuffer,
                                         d->lightTextureSize,
                                         d->lightTextureSize,
                                         CreateColorBuffer::No,
                                         Multisample::No,
                                         HDR::No,
                                         ExtendedFBO::No,
                                         true))
              return;

            DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers (B)");
            d->controller->setCurrentLight(light);
            DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers (C)");
            lightBuffer.worldToTexture = d->controller->lightMatrices();
            DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers (D)");

            if(light.castShadows)
              d->render();

            DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers (E)");
          }

          DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers");

          glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));
          glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
          DEBUG_printOpenGLError("RenderPass::LightFBOs bind default buffer");
#endif
          break;
        }

        case RenderPass::PrepareDrawFBO: //---------------------------------------------------------
        {
#ifdef TP_FBO_SUPPORTED
          DEBUG_scopedDebug("RenderPass::PrepareDrawFBO", TPPixel(0, 255, 0));
          d->renderInfo.hdr = hdr();
          d->renderInfo.extendedFBO = extendedFBO();
          glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
          d->currentDrawFBO = d->intermediateFBO(defaultSID());
          d->currentReadFBO = d->intermediateFBO(defaultSID());

          if(!d->buffers.prepareBuffer("currentDraw",
                                       *d->currentDrawFBO,
                                       d->width,
                                       d->height,
                                       CreateColorBuffer::Yes,
                                       Multisample::Yes,
                                       hdr(),
                                       extendedFBO(),
                                       true))
          {
            Errors::printOpenGLError("RenderPass::PrepareDrawFBO");
            return;
          }
#endif
          break;
        }

        case RenderPass::SwapToFBO: //--------------------------------------------------------------
        {
#ifdef TP_FBO_SUPPORTED
          DEBUG_scopedDebug("RenderPass::SwapToFBO " + renderPass.getNameString(), TPPixel(0, 0, 255));

          Multisample multisample = renderPass.index==0?Multisample::Yes:Multisample::No;

          d->renderInfo.hdr = hdr();
          d->renderInfo.extendedFBO = extendedFBO();
          d->buffers.swapMultisampledBuffer(*d->currentDrawFBO);
          d->currentReadFBO = d->currentDrawFBO;
          d->currentDrawFBO = d->intermediateFBO(renderPass.name);

          if(!d->buffers.prepareBuffer("currentDraw",
                                       *d->currentDrawFBO,
                                       d->width,
                                       d->height,
                                       CreateColorBuffer::Yes,
                                       multisample,
                                       hdr(),
                                       extendedFBO(),
                                       true))
          {
            Errors::printOpenGLError("RenderPass::SwapDrawFBO " + renderPass.getNameString());
            return;
          }
#endif
          break;
        }

        case RenderPass::SwapToFBONoClear: //-------------------------------------------------------
        {
#ifdef TP_FBO_SUPPORTED
          DEBUG_scopedDebug("RenderPass::SwapToFBONoClear " + renderPass.getNameString(), TPPixel(255, 255, 0));

          Multisample multisample = renderPass.name==defaultSID()?Multisample::Yes:Multisample::No;

          d->renderInfo.hdr = hdr();
          d->renderInfo.extendedFBO = extendedFBO();
          d->buffers.swapMultisampledBuffer(*d->currentDrawFBO);
          d->currentReadFBO = d->currentDrawFBO;
          d->currentDrawFBO = d->intermediateFBO(renderPass.name);

          if(!d->buffers.prepareBuffer("currentDraw",
                                       *d->currentDrawFBO,
                                       d->width,
                                       d->height,
                                       CreateColorBuffer::Yes,
                                       multisample,
                                       hdr(),
                                       extendedFBO(),
                                       false))
          {
            Errors::printOpenGLError("RenderPass::SwapDrawFBO " + renderPass.getNameString() + "NoClear");
            return;
          }
#endif
          break;
        }

        case RenderPass::BlitFromFBO: //------------------------------------------------------------
        {
#ifdef TP_FBO_SUPPORTED
          DEBUG_scopedDebug("RenderPass::BlitFromFBO " + renderPass.getNameString(), TPPixel(0, 255, 255));

          OpenGLFBO* readFBO = d->intermediateFBO(renderPass.name);

#ifdef TP_BLIT_WITH_SHADER
          // Kludge to get round a crash in glBlitFramebuffer on Chrome.
          {
            glDepthMask(false);
            glDisable(GL_DEPTH_TEST);

            auto shader = getShader<PostBlitShader>();
            if(shader->error())
              break;

            if(!d->rectangleObject)
              d->rectangleObject = shader->makeRectangleObject({1.0f,1.0f});

            shader->use(d->renderInfo.shaderType());
            shader->setReadFBO(*readFBO);

            glm::mat4 m{1.0f};
            shader->setFrameMatrix(m);
            shader->setProjectionMatrix(m);

            shader->draw(*d->rectangleObject);
          }
#else
          {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO->frameBuffer);

            glReadBuffer(GL_COLOR_ATTACHMENT0);
            DEBUG_printOpenGLError("RenderPass::BlitFromFBO " + renderPass.getNameString() + " a");

            d->buffers.setDrawBuffers({GL_COLOR_ATTACHMENT0});
            DEBUG_printOpenGLError("RenderPass::BlitFromFBO " + renderPass.getNameString() + " b");

            glBlitFramebuffer(0, 0, GLint(readFBO->width), GLint(readFBO->height), 0, 0, GLint(readFBO->width), GLint(readFBO->height), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

            DEBUG_printOpenGLError("RenderPass::BlitFromFBO " + renderPass.getNameString() + " blit color 0 and depth");

            if(readFBO->extendedFBO == ExtendedFBO::Yes)
            {
              glReadBuffer(GL_COLOR_ATTACHMENT1);
              d->buffers.setDrawBuffers({GL_COLOR_ATTACHMENT1});
              glBlitFramebuffer(0, 0, GLint(readFBO->width), GLint(readFBO->height), 0, 0, GLint(readFBO->width), GLint(readFBO->height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
              DEBUG_printOpenGLError("RenderPass::BlitFromFBO " + renderPass.getNameString() + " blit color 1");

              glReadBuffer(GL_COLOR_ATTACHMENT2);
              d->buffers.setDrawBuffers({GL_COLOR_ATTACHMENT2});
              glBlitFramebuffer(0, 0, GLint(readFBO->width), GLint(readFBO->height), 0, 0, GLint(readFBO->width), GLint(readFBO->height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
              DEBUG_printOpenGLError("RenderPass::BlitFromFBO " + renderPass.getNameString() + " blit color 2");

              d->buffers.setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
            }
            else
              d->buffers.setDrawBuffers({GL_COLOR_ATTACHMENT0});
          }
#endif

#endif
          break;
        }

        case RenderPass::Background: //-------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::Background", TPPixel(255, 0, 255));
          glDisable(GL_DEPTH_TEST);
          glDepthMask(GL_FALSE);
          d->render();
          break;
        }

        case RenderPass::Normal: //-----------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::Normal", TPPixel(125, 125, 0));
          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);
          glDepthMask(GL_TRUE);
          d->render();
          break;
        }

        case RenderPass::Transparency: //-----------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::Transparency", TPPixel(125, 125, 255));
          glEnable(GL_DEPTH_TEST);
          glDepthFunc(GL_LESS);
          glDepthMask(GL_TRUE);
          d->render();
          break;
        }

        case RenderPass::FinishDrawFBO: //----------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::FinishDrawFBO", TPPixel(50, 200, 255));
#ifdef TP_FBO_SUPPORTED
          d->renderInfo.hdr = HDR::No;
          d->renderInfo.extendedFBO = ExtendedFBO::No;
          d->buffers.swapMultisampledBuffer(*d->currentDrawFBO);
          std::swap(d->currentDrawFBO, d->currentReadFBO);
          glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
#endif
          break;
        }

        case RenderPass::Text: //-------------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::Text", TPPixel(200, 152, 255));
          glDisable(GL_DEPTH_TEST);
          glDepthMask(false);
          d->render();
          break;
        }

        case RenderPass::GUI3D: //------------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::GUI3D", TPPixel(200, 152, 50));
          glEnable(GL_DEPTH_TEST);
          auto s = pixelScale();
          glScissor(0, 0, GLsizei(float(width())*s), GLsizei(float(height())*s));
          glDepthMask(true);
          d->render();
          break;
        }

        case RenderPass::GUI: //--------------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::GUI", TPPixel(200, 152, 50));
          glEnable(GL_SCISSOR_TEST);
          glDisable(GL_DEPTH_TEST);
          auto s = pixelScale();
          glScissor(0, 0, GLsizei(float(width())*s), GLsizei(float(height())*s));
          glDepthMask(false);
          d->render();
          glDisable(GL_SCISSOR_TEST);
          break;
        }

        case RenderPass::Picking: //----------------------------------------------------------------
        case RenderPass::PickingGUI3D: //-----------------------------------------------------------
        {
          tpWarning() << "Error: Performing a picking render pass in paintGL does not make sense.";
          break;
        }

        case RenderPass::Custom: //-----------------------------------------------------------------
        {
          DEBUG_scopedDebug("RenderPass::Custom " + renderPass.getNameString(), TPPixel(90, 200, 100));
          d->render();
          break;
        }

        case RenderPass::Delegate: //---------------------------------------------------------------
        break;

        case RenderPass::Stage: //------------------------------------------------------------------
        break;
      }

      if(renderPass.postLayer)
      {
        DEBUG_scopedDebug("cleanupAfterRenderPass " + renderPass.getNameString(), TPPixel(90, 0, 150));
        renderPass.postLayer->cleanupAfterRenderPass(renderPass);
      }
    }
    catch (...)
    {
      tpWarning() << "Exception caught in  Map::paintGLNoMakeCurrent pass!";
      tpWarning() << "Pass: " << size_t(renderPass.type) <<
                     " name: " << renderPass.getNameString() <<
                     " rp: " << rp;
    }
  }

#ifdef TP_LINUX
#warning why
#endif
//  if(d->renderModeManager->renderMode() == RenderMode::Fast)
//  {
//    // switch off soft shadows for fast render
//    setShadowSamples(RenderMode::Fast, 0);
//  }
}

//##################################################################################################
void Map::resizeGL(int w, int h)
{
  makeCurrent();

  d->width  = size_t(w);
  d->height = size_t(h);

  glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));

  if(d->initialized)
    d->callMapResized();

  update();
}

//##################################################################################################
bool Map::mouseEvent(const MouseEvent& event)
{
  // If a layer or the controller has focus from a previous press event pass the release to it first.
  if(event.type == MouseEventType::Release || event.type == MouseEventType::Move)
  {
    for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
    {
      std::shared_ptr<EventHandler_lt> eventHandler=d->eventHandlers.at(i);
      if(auto i = eventHandler->m_hasMouseFocusFor.find(event.button); i!=eventHandler->m_hasMouseFocusFor.end())
      {
        if(event.type == MouseEventType::Release)
          eventHandler->m_hasMouseFocusFor.erase(i);

        if(eventHandler->callbacks.mouseEvent(event))
          return true;
      }
    }

    for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
    {
      Layer* layer = (*(--i));
      if(auto i = layer->m_hasMouseFocusFor.find(event.button); i!=layer->m_hasMouseFocusFor.end())
      {
        if(event.type == MouseEventType::Release)
          layer->m_hasMouseFocusFor.erase(i);

        if(layer->mouseEvent(event))
          return true;
      }
    }

    if(auto i = d->controller->m_hasMouseFocusFor.find(event.button); i!=d->controller->m_hasMouseFocusFor.end())
    {
      if(event.type == MouseEventType::Release)
        d->controller->m_hasMouseFocusFor.erase(i);

      if(d->controller->mouseEvent(event))
        return true;
    }
  }

  for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
  {
    std::shared_ptr<EventHandler_lt> eventHandler=d->eventHandlers.at(i);
    if(eventHandler->callbacks.mouseEvent(event))
    {
      if(event.type == MouseEventType::Press || event.type == MouseEventType::DragStart)
        eventHandler->m_hasMouseFocusFor.insert(event.button);
      return true;
    }
  }

  for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
  {
    Layer* layer = (*(--i));
    if(layer->mouseEvent(event))
    {
      if(event.type == MouseEventType::Press || event.type == MouseEventType::DragStart)
        layer->m_hasMouseFocusFor.insert(event.button);
      return true;
    }
  }

  if(d->controller->mouseEvent(event))
  {
    if(event.type == MouseEventType::Press || event.type == MouseEventType::DragStart)
      d->controller->m_hasMouseFocusFor.insert(event.button);
    return true;
  }

  if(event.type == MouseEventType::Press)
  {
    d->mouseEventHandler->press(event);
  }

  return false;
}

//##################################################################################################
bool Map::keyEvent(const KeyEvent& event)
{
  // If a layer or the controller has focus from a previous press event pass the release to it first.
  if(event.type == KeyEventType::Release)
  {
    for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
    {
      std::shared_ptr<EventHandler_lt> eventHandler=d->eventHandlers.at(i);
      if(auto i = eventHandler->m_hasKeyFocusFor.find(event.scancode); i!=eventHandler->m_hasKeyFocusFor.end())
      {
        eventHandler->m_hasKeyFocusFor.erase(i);
        if(eventHandler->callbacks.keyEvent(event))
          return true;
      }
    }

    for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
    {
      Layer* layer = (*(--i));
      if(auto i = layer->m_hasKeyFocusFor.find(event.scancode); i!=layer->m_hasKeyFocusFor.end())
      {
        layer->m_hasKeyFocusFor.erase(i);
        if(layer->keyEvent(event))
          return true;
      }
    }

    if(auto i = d->controller->m_hasKeyFocusFor.find(event.scancode); i!=d->controller->m_hasKeyFocusFor.end())
    {
      d->controller->m_hasKeyFocusFor.erase(i);
      if(d->controller->keyEvent(event))
        return true;
    }
  }

  for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
  {
    std::shared_ptr<EventHandler_lt> eventHandler=d->eventHandlers.at(i);
    if(eventHandler->callbacks.keyEvent(event))
    {
      if(event.type == KeyEventType::Press)
        eventHandler->m_hasKeyFocusFor.insert(event.scancode);
      return true;
    }
  }

  for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
  {
    Layer* layer = (*(--i));
    if(layer->keyEvent(event))
    {
      if(event.type == KeyEventType::Press)
        layer->m_hasKeyFocusFor.insert(event.scancode);
      return true;
    }
  }

  if(d->controller->keyEvent(event))
  {
    if(event.type == KeyEventType::Press)
      d->controller->m_hasKeyFocusFor.insert(event.scancode);
    return true;
  }

  return false;
}

//################################################################################################
bool Map::dragDropEvent(const DragDropEvent& event)
{
  for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
  {
    std::shared_ptr<EventHandler_lt> eventHandler=d->eventHandlers.at(i);
    if(eventHandler->callbacks.dragDropEvent(event))
      return true;
  }

  for(size_t i=d->layers.size()-1; i<d->layers.size(); i--)
    if(d->layers.at(i)->dragDropEvent(event))
      return true;

  return false;
}

//##################################################################################################
bool Map::textEditingEvent(const TextEditingEvent& event)
{
  for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
    if(d->eventHandlers.at(i)->callbacks.textEditingEvent(event))
      return true;

  for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
    if((*(--i))->textEditingEvent(event))
      return true;

  return false;
}

//##################################################################################################
bool Map::textInputEvent(const TextInputEvent& event)
{
  for(size_t i=d->eventHandlers.size()-1; i<d->eventHandlers.size(); i--)
    if(d->eventHandlers.at(i)->callbacks.textInputEvent(event))
      return true;

  for(auto i = d->layers.data() + d->layers.size(); i>d->layers.data();)
    if((*(--i))->textInputEvent(event))
      return true;

  return false;
}

//##################################################################################################
void Map::setRelativeMouseMode(bool enabled)
{
  TP_UNUSED(enabled);
}

//##################################################################################################
bool Map::relativeMouseMode() const
{
  return false;
}

//##################################################################################################
void Map::startTextInput()
{

}

//##################################################################################################
void Map::stopTextInput()
{

}

//##################################################################################################
void Map::layerDestroyed(Layer* layer)
{
  tpRemoveOne(d->layers, layer);
  update();
}

//##################################################################################################
void Map::setController(Controller* controller)
{
  delete d->controller;
  d->controller = controller;
  controllerUpdate();
}

//##################################################################################################
Shader* Map::getShader(const tp_utils::StringID& name) const
{
  return tpGetMapValue(d->shaders, name, nullptr);
}

//##################################################################################################
void Map::addShader(const tp_utils::StringID& name, Shader* shader)
{
  d->shaders[name] = shader;
}

//##################################################################################################
void Map::addFontRenderer(FontRenderer* fontRenderer)
{
  d->fontRenderers.push_back(fontRenderer);
}

//##################################################################################################
void Map::removeFontRenderer(FontRenderer* fontRenderer)
{
  tpRemoveOne(d->fontRenderers, fontRenderer);
}

//##################################################################################################
size_t Map::addEventHandler(int priority, Button hasMouseFocusFor)
{
  auto i = d->eventHandlers.begin();

  while(i!=d->eventHandlers.end() && (*i)->priority<=priority)
    ++i;

  auto& eventHandler = *d->eventHandlers.emplace(i, new EventHandler_lt);
  eventHandler->priority = priority;
  eventHandler->eventHandlerId = d->nextEventHandlerId;

  if(hasMouseFocusFor != Button::NoButton)
    eventHandler->m_hasMouseFocusFor.insert(hasMouseFocusFor);

  d->nextEventHandlerId++;

  return eventHandler->eventHandlerId;
}

//##################################################################################################
void Map::removeEventHandler(size_t eventHandlerId)
{
  for(auto i=d->eventHandlers.begin(); i!=d->eventHandlers.end(); ++i)
  {
    if((*i)->eventHandlerId == eventHandlerId)
    {
      d->eventHandlers.erase(i);
      break;
    }
  }
}

//##################################################################################################
void Map::updateEventHandlerCallbacks(size_t eventHandlerId,
                                      const std::function<void(EventHandlerCallbacks&)>& closure)
{
  for(auto i=d->eventHandlers.begin(); i!=d->eventHandlers.end(); ++i)
  {
    if((*i)->eventHandlerId == eventHandlerId)
    {
      closure((*i)->callbacks);
      break;
    }
  }
}

}
