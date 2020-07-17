#include "tp_maps/Map.h"
#include "tp_maps/Shader.h"
#include "tp_maps/Layer.h"
#include "tp_maps/controllers/FlatController.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/Texture.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/FontRenderer.h"

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/Ray.h"
#include "tp_math_utils/Intersection.h"

#include "tp_utils/StringID.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

namespace tp_maps
{

//##################################################################################################
struct Map::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Map::Private");
  TP_NONCOPYABLE(Private);

  Map* q;

  Controller* controller{nullptr};
  std::vector<Layer*> layers;
  std::unordered_map<tp_utils::StringID, Shader*> shaders;
  std::vector<FontRenderer*> fontRenderers;

  std::vector<RenderPass> renderPasses =
  {
    RenderPass::Background,
    RenderPass::Normal,
    RenderPass::Transparency,
    RenderPass::Text,
    RenderPass::GUI
  };

  // Callback that get called to prepare and cleanup custom render passes.
  std::function<void(RenderInfo&)> custom1Start;
  std::function<void(RenderInfo&)> custom1End;
  std::function<void(RenderInfo&)> custom2Start;
  std::function<void(RenderInfo&)> custom2End;
  std::function<void(RenderInfo&)> custom3Start;
  std::function<void(RenderInfo&)> custom3End;
  std::function<void(RenderInfo&)> custom4Start;
  std::function<void(RenderInfo&)> custom4End;

  RenderInfo renderInfo;

  int width{1};
  int height{1};
  glm::vec3 backgroundColor{0.0f, 0.0f, 0.0f};

  OpenGLProfile openGLProfile{TP_DEFAULT_PROFILE};

  FBO reflectionBuffer;

  std::vector<Light> lights{Light()};
  std::vector<FBO> lightTextures;

  FBO pickingBuffer;
  FBO renderToImageBuffer;

  bool initialized{false};
  bool preDeleteCalled{false};

  //################################################################################################
  Private(Map* q_):
    q(q_)
  {
    renderInfo.map = q;
  }

  //################################################################################################
  void render()
  {
    controller->updateMatrices();

    Layer** l = layers.data();
    Layer** lMax = l + layers.size();
    for(; l<lMax; l++)
      if((*l)->visible())
        (*l)->render(renderInfo);
  }

  //################################################################################################
  bool prepareBuffer(FBO& buffer, int width, int height)
  {
    if(buffer.width!=width || buffer.height!=height)
      deleteBuffer(buffer);

    if(!buffer.frameBuffer)
    {
      glGenFramebuffers(1, &buffer.frameBuffer);
      buffer.width  = width;
      buffer.height = height;
    }

    glBindFramebuffer(TP_GL_DRAW_FRAMEBUFFER, buffer.frameBuffer);

    if(!buffer.textureID)
    {
      glGenTextures(1, &buffer.textureID);
      glBindTexture(GL_TEXTURE_2D, buffer.textureID);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.textureID, 0);

      glGenTextures(1, &buffer.depthID);
      glBindTexture(GL_TEXTURE_2D, buffer.depthID);
      glTexImage2D(GL_TEXTURE_2D, 0, TP_GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, nullptr);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthID, 0);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
      tpWarning() << "Error Map::Private::prepareBuffer frame buffer not complete!";
      return false;
    }

    glViewport(0, 0, width, height);

    glDepthMask(true);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    return true;
  }

  //################################################################################################
  void deleteBuffer(FBO& buffer)
  {
    if(buffer.frameBuffer)
    {
      glDeleteFramebuffers(1, &buffer.frameBuffer);
      buffer.frameBuffer = 0;
    }

    if(buffer.textureID)
    {
      glDeleteTextures(1, &buffer.textureID);
      buffer.textureID = 0;
    }

    if(buffer.depthID)
    {
      glDeleteTextures(1, &buffer.depthID);
      buffer.depthID = 0;
    }
  }

  //################################################################################################
  void invalidateBuffer(FBO& buffer)
  {
    buffer.frameBuffer = 0;
    buffer.textureID   = 0;
    buffer.depthID     = 0;
  }
};

//##################################################################################################
Map::Map(bool):
  d(new Private(this))
{
  d->controller = new FlatController(this);
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
  makeCurrent();

  for(const auto& i : d->shaders)
    delete i.second;

  d->shaders.clear();

  delete d->controller;
  d->controller=nullptr;

  clearLayers();

  while(!d->fontRenderers.empty())
    delete tpTakeFirst(d->fontRenderers);

  d->deleteBuffer(d->reflectionBuffer);
  d->deleteBuffer(d->pickingBuffer);
  d->deleteBuffer(d->renderToImageBuffer);

  for(auto& lightBuffer : d->lightTextures)
    d->deleteBuffer(lightBuffer);

  d->preDeleteCalled = true;
}

//##################################################################################################
void Map::setOpenGLProfile(OpenGLProfile openGLProfile)
{
  d->openGLProfile = openGLProfile;
}

//##################################################################################################
OpenGLProfile Map::openGLProfile() const
{
  return d->openGLProfile;
}

//##################################################################################################
bool Map::initialized()const
{
  return d->initialized;
}

//##################################################################################################
void Map::printOpenGLError(const std::string& description)
{
  GLenum error = glGetError();

  if(error != GL_NO_ERROR)
  {
    std::string errorString;
    switch(error)
    {
    case GL_INVALID_ENUM      : errorString = "GL_INVALID_ENUM"      ; break;
    case GL_INVALID_VALUE     : errorString = "GL_INVALID_VALUE"     ; break;
    case GL_INVALID_OPERATION : errorString = "GL_INVALID_OPERATION" ; break;
    case GL_OUT_OF_MEMORY     : errorString = "GL_OUT_OF_MEMORY"     ; break;
    default: break;
    }
    tpWarning() << description << " OpenGL Error: " << errorString << "(" << error << ")";
  }
}

//##################################################################################################
RenderInfo& Map::renderInfo()
{
  return d->renderInfo;
}

//##################################################################################################
void Map::animate(double timestampMS)
{
  d->controller->animate(timestampMS);

  Layer** l = d->layers.data();
  Layer** lMax = l + d->layers.size();
  for(; l<lMax; l++)
    (*l)->animate(timestampMS);
}

//##################################################################################################
Controller* Map::controller()
{
  return d->controller;
}

//##################################################################################################
void Map::setBackgroundColor(const glm::vec3& color)
{
  d->backgroundColor = color;

  if(d->initialized)
  {
    makeCurrent();
    glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, 1.0f);
  }

  update();
}

//##################################################################################################
glm::vec3 Map::backgroundColor()const
{
  return d->backgroundColor;
}

//##################################################################################################
void Map::setRenderPasses(const std::vector<RenderPass>& renderPasses)
{
  d->renderPasses = renderPasses;
  update();
}

//##################################################################################################
const std::vector<RenderPass>& Map::renderPasses() const
{
  return d->renderPasses;
}

//##################################################################################################
void Map::setCustomRenderPass(RenderPass renderPass,
                              const std::function<void(RenderInfo&)>& start,
                              const std::function<void(RenderInfo&)>& end)
{
  switch(renderPass)
  {
  case RenderPass::Custom1:
    d->custom1Start = start;
    d->custom1End   = end;
    break;

  case RenderPass::Custom2:
    d->custom2Start = start;
    d->custom2End   = end;
    break;

  case RenderPass::Custom3:
    d->custom3Start = start;
    d->custom3End   = end;
    break;

  case RenderPass::Custom4:
    d->custom4Start = start;
    d->custom4End   = end;
    break;

  default:
    break;
  }
}

//##################################################################################################
void Map::setLights(const std::vector<Light>& lights)
{
  d->lights = lights;
  update();
}

//##################################################################################################
const std::vector<Light>& Map::lights()const
{
  return d->lights;
}

//##################################################################################################
void Map::addLayer(Layer* layer)
{
  insertLayer(d->layers.size(), layer);
}

//##################################################################################################
void Map::insertLayer(size_t i, Layer *layer)
{
  if(layer->map())
  {
    tpWarning() << "Error! Map::insertLayer inserting a layer that is already in a map!";
    tp_utils::printStackTrace();
    return;
  }

  d->layers.insert(d->layers.begin()+int(i), layer);
  layer->setMap(this);
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
const std::vector<Layer*>& Map::layers()const
{
  return d->layers;
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
  v /= v.w;

  v = (v+1.0f)/2.0f;

  screenPoint.x = v.x * float(d->width);
  screenPoint.y = float(d->height) - (v.y * float(d->height));
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

//################################################################################################
glm::vec3 Map::unProject(const glm::vec3& screenPoint)
{
  return unProject(screenPoint, d->controller->matrix(defaultSID()));
}

//################################################################################################
glm::vec3 Map::unProject(const glm::vec3& screenPoint, const glm::mat4& matrix)
{
  glm::mat4 inverse = glm::inverse(matrix);

  glm::vec4 tmp{screenPoint, 1.0f};
  tmp.x = tmp.x / float(d->width);
  tmp.y = (float(d->height) - tmp.y) / float(d->height);
  tmp = tmp * 2.0f - 1.0f;
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

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  //------------------------------------------------------------------------------------------------
  // Configure the frame buffer that the picking values will be rendered to.
  if(!d->prepareBuffer(d->renderToImageBuffer, d->width, d->height))
    return nullptr;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, 1.0f);

  //------------------------------------------------------------------------------------------------
  // Execute a picking render pass.
  d->renderInfo.resetPicking();
  d->renderInfo.pass = RenderPass::Picking;
  d->renderInfo.pickingType = pickingType;
  d->renderInfo.pos = pos;

  d->render();

  //------------------------------------------------------------------------------------------------
  // Read the small patch from around the picking position and then free up the frame buffers.

  //The size of the area to perform picking in, must be an odd number
  int windowX = tpBound(0, pos.x-left, width()-(pickingSize+1));
  int windowY = tpBound(0, (d->height-pos.y)-left, height()-(pickingSize+1));
  std::vector<unsigned char> pixels(pickingSize*pickingSize*4);
  glReadPixels(windowX, windowY, pickingSize, pickingSize, GL_RGBA, GL_UNSIGNED_BYTE, &pixels[0]);

  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);

  //------------------------------------------------------------------------------------------------
  // Iterate over the 9x9 patch of picking data and find the most appropriate result. We favor
  // results near the center of the the patch.

  static const std::vector<glm::ivec2> testOrder(generateTestOrder(pickingSize));
  for(const auto& point : testOrder)
  {

    unsigned char* p = pixels.data() + (((point.y*9)+point.x)*4);

    uint32_t r = p[0];
    uint32_t g = p[1];
    uint32_t b = p[2];

    uint32_t value = r;
    value |= (g<<8);
    value |= (b<<16);

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
                pickingDetails.callback(PickingResult(pickingType, pickingDetails, d->renderInfo)):
                nullptr;
        }
      }
    }
  }

  return nullptr;
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, std::vector<TPPixel>& pixels, bool swapY)
{
  pixels.resize(size_t(width*height));
  return renderToImage(width, height, pixels.data(), swapY);
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, TPPixel* pixels, bool swapY)
{
  if(width<1 || height<1)
  {
    tpWarning() << "Error Map::renderToImage can't render to image smaller than 1 pixel.";
    return false;
  }

  auto originalWidth  = d->width;
  auto originalHeight = d->height;

  makeCurrent();

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  // Configure the frame buffer that the image will be rendered to.
  if(!d->prepareBuffer(d->renderToImageBuffer, int(width), int(height)))
    return false;

  // Execute a render passes.
  paintGLNoMakeCurrent();

  // Read the texture that we just generated
  glReadPixels(0, 0, int(width), int(height), GL_RGBA, GL_UNSIGNED_BYTE, pixels);

  if(swapY)
  {
    std::vector<TPPixel> line{size_t(width)};
    TPPixel* c = line.data();
    size_t rowLengthBytes = size_t(width)*sizeof(TPPixel);
    size_t yMax = size_t(height)/2;
    for(size_t y=0; y<yMax; y++)
    {
      TPPixel* a{pixels + y*size_t(width)};
      TPPixel* b{pixels + (size_t(height-1)-y)*size_t(width)};

      memcpy(c, a, rowLengthBytes);
      memcpy(a, b, rowLengthBytes);
      memcpy(b, c, rowLengthBytes);
    }
  }

  // Return to the original viewport settings
  d->width  = originalWidth;
  d->height = originalHeight;
  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);
  glViewport(0, 0, d->width, d->height);

  return true;
}

//##################################################################################################
void Map::deleteTexture(GLuint id)
{
  if(id>0)
    glDeleteTextures(1, &id);
}

//##################################################################################################
GLuint Map::reflectionTexture() const
{
  return d->reflectionBuffer.textureID;
}

//##################################################################################################
GLuint Map::reflectionDepth() const
{
  return d->reflectionBuffer.depthID;
}

//##################################################################################################
const std::vector<FBO>& Map::lightTextures() const
{
  return d->lightTextures;
}

//##################################################################################################
int Map::width() const
{
  return d->width;
}

//##################################################################################################
int Map::height() const
{
  return d->height;
}

//##################################################################################################
glm::vec2 Map::screenSize() const
{
  return {d->width, d->height};
}

//##################################################################################################
float Map::pixelScale()
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

  //Invalidate old state before initializing new state
  {
    for(auto i : d->shaders)
    {
      i.second->invalidate();
      delete i.second;
    }
    d->shaders.clear();

    d->invalidateBuffer(d->reflectionBuffer);
    d->invalidateBuffer(d->pickingBuffer);
    d->invalidateBuffer(d->renderToImageBuffer);
    d->lightTextures.clear();

    for(auto i : d->layers)
      i->invalidateBuffers();

    for(auto i : d->fontRenderers)
      i->invalidateBuffers();
  }

  // Initialize GL
  // On some platforms the context isn't current, so fix that first
  makeCurrent();

  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DITHER);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, 1.0f);
  d->initialized = true;

  d->controller->mapResized(d->width, d->height);
}

//##################################################################################################
void Map::paintGL()
{
  makeCurrent();
  paintGLNoMakeCurrent();
}

//##################################################################################################
void Map::paintGLNoMakeCurrent()
{
#ifdef TP_FBO_SUPPORTED
  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
#endif

#if 0
  //Make the background flicker to show when the map is updateing
  glClearColor(1.0f, 1.0f, float(std::rand()%255)/255.0f, 1.0f);
#endif
  glDepthMask(true);
  glClearDepthf(1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for(auto renderPass : d->renderPasses)
  {
    d->renderInfo.pass = renderPass;
    switch(renderPass)
    {
    case RenderPass::LightFBOs: //------------------------------------------------------------------
    {
#ifdef TP_FBO_SUPPORTED
      while(d->lightTextures.size() < d->lights.size())
        d->lightTextures.emplace_back();

      while(d->lightTextures.size() > d->lights.size())
      {
        auto buffer = tpTakeLast(d->lightTextures);
        d->deleteBuffer(buffer);
      }

      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(true);

      for(size_t i=0; i< d->lightTextures.size(); i++)
      {
        const auto& light = d->lights.at(i);
        auto& lightBuffer = d->lightTextures.at(i);

        if(!d->prepareBuffer(lightBuffer, 1024, 1024))
          return;

        d->controller->setCurrentLight(light);
        d->render();
      }
      glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
#endif
      break;
    }

    case RenderPass::ReflectionFBO: //--------------------------------------------------------------
    {
#ifdef TP_FBO_SUPPORTED
      glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
      if(!d->prepareBuffer(d->reflectionBuffer, d->width, d->height))
        return;
#endif
      break;
    }

    case RenderPass::Background: //-----------------------------------------------------------------
    {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
      d->render();
      break;
    }

    case RenderPass::Normal: //---------------------------------------------------------------------
    {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(true);
      d->render();
      break;
    }

    case RenderPass::Transparency: //---------------------------------------------------------------
    {
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(false);
      d->render();
      break;
    }

    case RenderPass::Reflection: //-----------------------------------------------------------------
    {
#ifdef TP_FBO_SUPPORTED
      glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
      glBindFramebuffer(GL_READ_FRAMEBUFFER, d->reflectionBuffer.frameBuffer);

#if 1
      glBlitFramebuffer(0, 0, d->width, d->height,
                        0, 0, d->width, d->height,
                        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
                        GL_NEAREST);
#else
      glBlitFramebuffer(0, 0, d->width, d->height,
                        0, 0, d->width, d->height,
                        GL_DEPTH_BUFFER_BIT,
                        GL_NEAREST);

      glBlitFramebuffer(0, 0, d->width, d->height,
                        0, 0, d->width, d->height,
                        GL_COLOR_BUFFER_BIT ,
                        GL_LINEAR);
#endif

      // Call this again to replace GL_READ_FRAMEBUFFER
      glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));

      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(false);
      d->render();
#endif
      break;
    }

    case RenderPass::Text: //-----------------------------------------------------------------------
    {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
      d->render();
      break;
    }

    case RenderPass::GUI: //------------------------------------------------------------------------
    {
      glEnable(GL_SCISSOR_TEST);
      glDisable(GL_DEPTH_TEST);
      auto s = pixelScale();
      glScissor(0, 0, GLsizei(float(width())*s), GLsizei(float(height())*s));
      glDepthMask(false);
      d->render();
      glDisable(GL_SCISSOR_TEST);
      break;
    }

    case RenderPass::Picking: //--------------------------------------------------------------------
    {
      tpWarning() << "Error: Performing a picking render pass in paintGL does not make sense.";
      break;
    }

    case RenderPass::Custom1: //--------------------------------------------------------------------
    {
      if(d->custom1Start)
        d->custom1Start(d->renderInfo);

      d->render();

      if(d->custom1End)
        d->custom1End(d->renderInfo);

      break;
    }

    case RenderPass::Custom2: //--------------------------------------------------------------------
    {
      if(d->custom2Start)
        d->custom2Start(d->renderInfo);

      d->render();

      if(d->custom2End)
        d->custom2End(d->renderInfo);

      break;
    }

    case RenderPass::Custom3: //--------------------------------------------------------------------
    {
      if(d->custom3Start)
        d->custom3Start(d->renderInfo);

      d->render();

      if(d->custom3End)
        d->custom3End(d->renderInfo);

      break;
    }

    case RenderPass::Custom4: //--------------------------------------------------------------------
    {
      if(d->custom4Start)
        d->custom4Start(d->renderInfo);

      d->render();

      if(d->custom4End)
        d->custom4End(d->renderInfo);

      break;
    }
    }
  }

  printOpenGLError("Map::paintGL");
}

//##################################################################################################
void Map::resizeGL(int w, int h)
{
  makeCurrent();

  d->width  = w;
  d->height = h;

  glViewport(0, 0, d->width, d->height);

  if(d->initialized)
    d->controller->mapResized(w, h);

  update();
}

//##################################################################################################
bool Map::mouseEvent(const MouseEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->mouseEvent(event))
      return true;
  return d->controller->mouseEvent(event);
}

//##################################################################################################
bool Map::keyEvent(const KeyEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->keyEvent(event))
      return true;
  return d->controller->keyEvent(event);
}

//##################################################################################################
bool Map::textEditingEvent(const TextEditingEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->textEditingEvent(event))
      return true;
  return false;
}

//##################################################################################################
bool Map::textInputEvent(const TextInputEvent& event)
{
  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    if((*(--l))->textInputEvent(event))
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
void Map::mapLayerDestroyed(Layer* layer)
{
  tpRemoveOne(d->layers, layer);
}

//##################################################################################################
void Map::setController(Controller* controller)
{
  delete d->controller;
  d->controller = controller;
}

//##################################################################################################
Shader* Map::getShader(const tp_utils::StringID& name)const
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

}
