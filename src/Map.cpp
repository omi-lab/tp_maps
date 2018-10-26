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
  Map* q;

  Controller* controller{nullptr};
  std::vector<Layer*> layers;  
  std::unordered_map<tp_utils::StringID, Shader*> shaders;
  std::vector<FontRenderer*> fontRenderers;

  RenderInfo renderInfo;

  int width{1};
  int height{1};
  glm::vec3 backgroundColor{0.0f, 0.0f, 0.0f};

  bool enableDepthBuffer;
  bool initialized{false};
  bool preDeleteCalled{false};

  //################################################################################################
  Private(Map* q_, bool enableDepthBuffer_):
    q(q_),
    enableDepthBuffer(enableDepthBuffer_)
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
  void initDepthBuffer()
  {
    if(enableDepthBuffer)
    {
      glEnable(GL_DEPTH_TEST);
      glClearDepthf(1.0f);
      glDepthFunc(GL_LESS);
    }
    else
      glDisable(GL_DEPTH_TEST);
  }
};

//##################################################################################################
Map::Map(bool enableDepthBuffer):
  d(new Private(this, enableDepthBuffer))
{
  //Create a new controller, it will assign itself to the map.
  Controller* controller = new FlatController(this);
  TP_UNUSED(controller);
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

  for(auto i : d->shaders)
    delete i.second;

  d->shaders.clear();

  delete d->controller;
  d->controller=nullptr;

  clearLayers();

  tpDeleteAll(d->fontRenderers);
  d->fontRenderers.clear();

  d->preDeleteCalled = true;
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
    glClearColor(d->backgroundColor.r, d->backgroundColor.g, d->backgroundColor.b, 1.0f);
  }

  update();
}

//##################################################################################################
glm::vec3 Map::backgroundColor()const
{
  return d->backgroundColor;
}

//##################################################################################################
void Map::setEnableDepthBuffer(bool enableDepthBuffer)
{
  d->enableDepthBuffer = enableDepthBuffer;
  if(d->initialized)
  {
    makeCurrent();
    d->initDepthBuffer();
  }
}

//##################################################################################################
bool Map::enableDepthBuffer()const
{
  return d->enableDepthBuffer;
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
bool Map::unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane)
{
  return unProject(screenPoint, scenePoint, plane, d->controller->matrix(defaultSID()));
}

//##################################################################################################
bool Map::unProject(const glm::vec2& screenPoint, glm::vec3& scenePoint, const tp_math_utils::Plane& plane, const glm::mat4& matrix)
{
  glm::mat4 inverse = glm::inverse(matrix);

  glm::vec4 screenPoints[2];
  glm::vec3 scenePoints[2];

  screenPoints[0] = glm::vec4(screenPoint, 0.0f, 1.0f);
  screenPoints[1] = glm::vec4(screenPoint, 1.0f, 1.0f);

  for(int i=0; i<2; i++)
  {
    glm::vec4& tmp = screenPoints[i];
    tmp.x = tmp.x / float(d->width);
    tmp.y = (d->height - tmp.y) / float(d->height);
    tmp = tmp * 2.0f - 1.0f;

    glm::vec4 obj = inverse * tmp;
    obj /= obj.w;
    scenePoints[i]= glm::vec3(obj);
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
  tmp.y = (d->height - tmp.y) / float(d->height);
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

  if(width()<pickingSize || height()<pickingSize)
    return nullptr;

  makeCurrent();

  //------------------------------------------------------------------------------------------------
  // Configure the frame buffer that the picking values will be rendered to.
  GLuint frameBuffer = 0;
  glGenFramebuffers(1, &frameBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

  GLuint frameBufferTexture = 0;
  glGenTextures(1, &frameBufferTexture);
  glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, d->width, d->height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  GLuint frameBufferDepth = 0;
  glGenRenderbuffers(1, &frameBufferDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, frameBufferDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, d->width, d->height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, frameBufferDepth);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBufferTexture, 0);

  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    tpWarning() << "Error Map::performPicking frame buffer not complete!";

    if(frameBuffer)
      glDeleteFramebuffers(1, &frameBuffer);

    if(frameBufferTexture)
      glDeleteTextures(1, &frameBufferTexture);

    if(frameBufferDepth)
      glDeleteRenderbuffers(1, &frameBufferDepth);

    return nullptr;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
  glViewport(0, 0, d->width, d->height);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(d->backgroundColor.g, d->backgroundColor.g, d->backgroundColor.b, 1.0f);


  //------------------------------------------------------------------------------------------------
  // Execute a picking render pass.
  d->renderInfo.resetPicking();
  d->renderInfo.pass = PickingRenderPass;
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

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDeleteFramebuffers(1, &frameBuffer);
  glDeleteTextures(1, &frameBufferTexture);
  glDeleteRenderbuffers(1, &frameBufferDepth);


  //------------------------------------------------------------------------------------------------
  // Iterate over the 9x9 patch of picking data and find the most appropriate result. We favor
  // results near the center of the the patch.

  static const std::vector<glm::ivec2> testOrder(generateTestOrder(pickingSize));
  for(size_t i=0; i<testOrder.size(); i++)
  {
    const glm::ivec2& point = testOrder.at(i);
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
void Map::deleteTexture(GLuint id)
{
  if(id>0)
    glDeleteTextures(1, &id);
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
glm::vec2 Map::screenSize()const
{
  return {d->width, d->height};
}

//##################################################################################################
void Map::initializeGL()
{
  //Invalidate old state before initializing new state
  {
    for(auto i : d->shaders)
    {
      i.second->invalidate();
      delete i.second;
    }
    d->shaders.clear();

    for(auto i : d->layers)
      i->invalidateBuffers();

    for(auto i : d->fontRenderers)
      i->invalidateBuffers();
  }

  // Initialize GL
  // On some platforms the context isn't current, so fix that first
  makeCurrent();

  d->initDepthBuffer();

  glDisable(GL_SCISSOR_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_DITHER);

  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);

  glClearColor(d->backgroundColor.r, d->backgroundColor.g, d->backgroundColor.b, 1.0f);
  d->initialized = true;

  d->controller->mapResized(d->width, d->height);
}

//##################################################################################################
void Map::paintGL()
{
  makeCurrent();
#if 0
  //Make the background flicker to show when the map is updateing
  glClearColor(1.0f, 1.0f, float(std::rand()%255)/255.0f, 1.0f);
#endif
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  d->renderInfo.pass = NormalRenderPass;
  d->render();
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
