#include "tp_maps/Map.h"
#include "tp_maps/Shader.h"
#include "tp_maps/Layer.h"
#include "tp_maps/controllers/FlatController.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/Texture.h"
#include "tp_maps/MouseEvent.h"
#include "tp_maps/KeyEvent.h"
#include "tp_maps/FontRenderer.h"
#include "tp_maps/textures/DefaultSpritesTexture.h"

#include "tp_math_utils/Plane.h"
#include "tp_math_utils/Ray.h"
#include "tp_math_utils/Intersection.h"

#include "tp_utils/StringID.h"
#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"
#include "tp_utils/StackTrace.h"

#include "glm/glm.hpp"
#include "glm/gtx/norm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#ifdef TP_MAPS_DEBUG
#  define DEBUG_printOpenGLError(A) printOpenGLError(A)
#else
#  define DEBUG_printOpenGLError(A) do{}while(false)
#endif

#if defined(TP_MAPS_DEBUG) && !defined(TP_EMSCRIPTEN) && !defined(TP_OSX)
//##################################################################################################
static void APIENTRY tpOutputOpenGLDebug(GLenum source,
                                         GLenum type,
                                         unsigned int id,
                                         GLenum severity,
                                         GLsizei length,
                                         const char *message,
                                         const void *userParam)
{
  TP_UNUSED(length);
  TP_UNUSED(userParam);

  // ignore non-significant error/warning codes
  if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

  tpWarning() << "--------------- tpOutputOpenGLDebug ---------------";
  tpWarning() << "OpenGL Debug message (" << id << "): " <<  message << std::endl;

  switch(source)
  {
  case GL_DEBUG_SOURCE_API:             tpWarning() << "  Source: API"; break;
  case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   tpWarning() << "  Source: Window System"; break;
  case GL_DEBUG_SOURCE_SHADER_COMPILER: tpWarning() << "  Source: Shader Compiler"; break;
  case GL_DEBUG_SOURCE_THIRD_PARTY:     tpWarning() << "  Source: Third Party"; break;
  case GL_DEBUG_SOURCE_APPLICATION:     tpWarning() << "  Source: Application"; break;
  case GL_DEBUG_SOURCE_OTHER:           tpWarning() << "  Source: Other"; break;
  }

  switch(type)
  {
  case GL_DEBUG_TYPE_ERROR:               tpWarning() << "  Type: Error"; break;
  case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: tpWarning() << "  Type: Deprecated Behaviour"; break;
  case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  tpWarning() << "  Type: Undefined Behaviour"; break;
  case GL_DEBUG_TYPE_PORTABILITY:         tpWarning() << "  Type: Portability"; break;
  case GL_DEBUG_TYPE_PERFORMANCE:         tpWarning() << "  Type: Performance"; break;
  case GL_DEBUG_TYPE_MARKER:              tpWarning() << "  Type: Marker"; break;
  case GL_DEBUG_TYPE_PUSH_GROUP:          tpWarning() << "  Type: Push Group"; break;
  case GL_DEBUG_TYPE_POP_GROUP:           tpWarning() << "  Type: Pop Group"; break;
  case GL_DEBUG_TYPE_OTHER:               tpWarning() << "  Type: Other"; break;
  }

  switch(severity)
  {
  case GL_DEBUG_SEVERITY_HIGH:         tpWarning() << "  Severity: high"; break;
  case GL_DEBUG_SEVERITY_MEDIUM:       tpWarning() << "  Severity: medium"; break;
  case GL_DEBUG_SEVERITY_LOW:          tpWarning() << "  Severity: low"; break;
  case GL_DEBUG_SEVERITY_NOTIFICATION: tpWarning() << "  Severity: notification"; break;
  }

  tpWarning() << "Error stack trace:";
  tp_utils::printStackTrace();
  tpWarning() << "---------------\n";
}
#endif

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
    RenderPass::Background  ,
    RenderPass::Normal      ,
    RenderPass::Transparency,
    RenderPass::Text        ,
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

  size_t width{1};
  size_t height{1};
  glm::vec3 backgroundColor{0.0f, 0.0f, 0.0f};

  OpenGLProfile openGLProfile{TP_DEFAULT_PROFILE};
  bool visible{true};

  // The first is multisampled the second and third are not.
  // We don't want to multisample multiple times it just makes the result blury. So what we do here
  // is have 3 buffers, the first has the 3D geometry drawn to it and is multisampled after this the
  // render pipeline toggles between the second and third for the remaining post processing steps.
  FBO intermediateFBOs[3];

  FBO* currentReadFBO{&intermediateFBOs[0]};
  FBO* currentDrawFBO{&intermediateFBOs[0]};

  std::vector<tp_math_utils::Light> lights;
  std::vector<FBO> lightBuffers;
  size_t lightTextureSize{1024};
  size_t spotLightLevels{1};
  size_t shadowSamples{0};

  HDR hdr{HDR::No};

  bool updateSamplesRequired{true};
  size_t maxSamples{1};
  size_t samples{1};

  Texture* spotLightTexture{nullptr};
  GLuint spotLightTextureID{0};

  FBO pickingBuffer;
  FBO renderToImageBuffer;

  bool initialized{false};
  bool preDeleteCalled{false};

  //################################################################################################
  Private(Map* q_):
    q(q_)
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
    q->makeCurrent();

    for(const auto& i : shaders)
      delete i.second;

    shaders.clear();
  }

  //################################################################################################
  void render()
  {
    controller->updateMatrices();

    for(auto l : layers)
      if(l->visible())
        l->render(renderInfo);
  }

  //################################################################################################
  GLint colorFormatF(Alpha alpha)
  {
    switch(openGLProfile)
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB16F;

    default:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB32F;
    }
  }

  //################################################################################################
  GLint depthFormatF()
  {
    switch(openGLProfile)
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      return GL_R32F;

    default:
      return GL_R32F;
    }
  }

  //################################################################################################
  void create2DColorTexture(GLuint& textureID, size_t width, size_t height, HDR hdr, Alpha alpha)
  {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    DEBUG_printOpenGLError("create2DColorTexture A");

    if(hdr == HDR::No)
    {
      if(alpha == Alpha::No)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TPGLsizei(width), TPGLsizei(height), 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TPGLsizei(width), TPGLsizei(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
      DEBUG_printOpenGLError("create2DColorTexture B");
    }
    else
    {
      if(alpha == Alpha::No)
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGB, GL_FLOAT, nullptr);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGBA, GL_FLOAT, nullptr);
      DEBUG_printOpenGLError("create2DColorTexture C");
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    DEBUG_printOpenGLError("prepareBuffer generate 2D texture for color buffer");
  }

#ifdef TP_ENABLE_MULTISAMPLE_FBO
  //################################################################################################
  void createMultisampleTexture(GLuint& multisampleTextureID, size_t width, size_t height, HDR hdr, Alpha alpha, GLenum attachment)
  {
    switch(openGLProfile)
    {
    case OpenGLProfile::VERSION_100_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      break;

    default:
      glGenTextures(1, &multisampleTextureID);
      glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureID);

      if(hdr == HDR::No)
      {
        if(alpha == Alpha::No)
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), GL_RGB, TPGLsizei(width), TPGLsizei(height), GL_TRUE);
        else
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), GL_RGBA, TPGLsizei(width), TPGLsizei(height), GL_TRUE);
      }
      else
      {
        if(alpha == Alpha::No)
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), GL_TRUE);
        else
          glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), GL_TRUE);
      }

      glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D_MULTISAMPLE, multisampleTextureID, 0);
      break;
    }
    DEBUG_printOpenGLError("prepareBuffer generate 2D texture for multisample FBO");
  }
#endif

  //################################################################################################
  void create2DDepthTexture(GLuint& depthID, size_t width, size_t height)
  {
    glGenTextures(1, &depthID);

    glBindTexture(GL_TEXTURE_2D, depthID);

    switch(openGLProfile)
    {
    case OpenGLProfile::VERSION_100_ES:
      glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;

    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      glTexImage2D(GL_TEXTURE_2D, 0, TP_GL_DEPTH_COMPONENT24, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
      break;

    default:
      glTexImage2D(GL_TEXTURE_2D, 0, TP_GL_DEPTH_COMPONENT24, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    DEBUG_printOpenGLError("prepareBuffer generate 2D texture for depth buffer");
  }

  //################################################################################################
  void create3DDepthTexture(GLuint& depthID, size_t width, size_t height, size_t levels)
  {
    glGenTextures(1, &depthID);

    glBindTexture(GL_TEXTURE_3D, depthID);

    switch(openGLProfile)
    {
    case OpenGLProfile::VERSION_100_ES:
      glTexImage3D(GL_TEXTURE_3D, 0, GL_DEPTH_COMPONENT, TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, nullptr);
      break;

    case OpenGLProfile::VERSION_300_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_310_ES: [[fallthrough]];
    case OpenGLProfile::VERSION_320_ES:
      glTexImage3D(GL_TEXTURE_3D, 0, depthFormatF(), TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_RED, GL_FLOAT, nullptr);
      break;

    default:
      glTexImage3D(GL_TEXTURE_3D, 0, depthFormatF(), TPGLsizei(width), TPGLsizei(height), TPGLsizei(levels), 0, GL_RED, GL_UNSIGNED_SHORT, nullptr);
      break;
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    DEBUG_printOpenGLError("prepareBuffer generate 3D texture for depth buffer");
  }

  //################################################################################################
  void createColorRBO(GLuint& rboID, size_t width, size_t height, HDR hdr, Alpha alpha, GLenum attachment)
  {
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);

    if(hdr == HDR::No)
    {
      if(alpha == Alpha::No)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), GL_RGB8, TPGLsizei(width), TPGLsizei(height));
      else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), GL_RGBA8, TPGLsizei(width), TPGLsizei(height));
    }
    else
    {
      if(alpha == Alpha::No)
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height));
      else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height));
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    DEBUG_printOpenGLError("prepareBuffer multisample RBO for attachment " + std::to_string(attachment));
  }

  //################################################################################################
  void createDepthRBO(GLuint& rboID, size_t width, size_t height)
  {
    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), TP_GL_DEPTH_COMPONENT24, TPGLsizei(width), TPGLsizei(height));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
    DEBUG_printOpenGLError("prepareBuffer multisample RBO for depth");
  }

  //################################################################################################
  void setDrawBuffers(const std::vector<GLenum>& buffers)
  {
    switch(openGLProfile)
    {
    default:
      glDrawBuffers(TPGLsizei(buffers.size()), buffers.data());
      break;

    case OpenGLProfile::VERSION_100_ES:
      break;
    }

  }

  //################################################################################################
  //! Create and bind an FBO.
  /*!
  This will delete and recreate the buffer if required.

  \param buffer holds the resource ID's generated by this function.
  \param width of the buffer pixels.
  \param height of the buffer in pixels.
  \param createColorBuffer should be true if we need to create a color buffer, false for shadows.
  \param multisample should be true to enable antialiasing (MSAA).
  \param levels controls the number of textures to generate for a shadow texture, else 1.
  \param level index to bind when rendering lights.

  \return true if we managed to create a functional FBO.
  */
  bool prepareBuffer(FBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     size_t levels,
                     size_t level)
  {
    DEBUG_printOpenGLError("prepareBuffer Start");

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(updateSamplesRequired)
    {
      updateSamplesRequired = false;

      GLint max=1;
      glGetIntegerv(GL_MAX_SAMPLES, &max);
      samples = tpMin(size_t(max), maxSamples);

      if(samples != maxSamples)
        tpWarning() << "Max samples set to: " << samples;
    }
#endif

#ifndef TP_ENABLE_3D_TEXTURE
    levels = 1;
#endif

    multisample = (multisample==Multisample::Yes && (samples>1))?Multisample::Yes:Multisample::No;

    if(buffer.width!=width || buffer.height!=height || buffer.levels!=levels || buffer.samples!=samples || buffer.hdr != hdr || buffer.multisample != multisample)
      deleteBuffer(buffer);

    buffer.level = level;

    if(!buffer.frameBuffer)
    {
      glGenFramebuffers(1, &buffer.frameBuffer);
      buffer.width       = width;
      buffer.height      = height;
      buffer.levels      = levels;
      buffer.samples     = samples;
      buffer.hdr         = hdr;
      buffer.multisample = multisample;
    }

    DEBUG_printOpenGLError("prepareBuffer Init");

    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer first");

    // Some versions of OpenGL must have a color buffer even if we are not going to use it.
    if(createColorBuffer == CreateColorBuffer::No)
    {
      if(openGLProfile == OpenGLProfile::VERSION_100_ES)
        createColorBuffer = CreateColorBuffer::Yes;
    }

#ifdef TP_ENABLE_3D_TEXTURE
    if(levels != 1)
      createColorBuffer = CreateColorBuffer::No;
#endif

    if(createColorBuffer == CreateColorBuffer::Yes)
    {
      if(!buffer.textureID)
        create2DColorTexture(buffer.textureID, width, height, hdr, Alpha::No);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.textureID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO");
    }

#ifdef TP_ENABLE_3D_TEXTURE
    if(levels != 1)
    {
      //Its not possible to bind a 3D texture as a depth buffer, so we bind it as the color buffer
      //and copy the depth values to that color buffer. We do however still require an actual depth
      //buffer to perform depth tests against. So here textureID gets prepared as a 2D depth buffer.
      //The 2D depth buffer is bound as GL_DEPTH_ATTACHMENT.
      //The 3D depth buffer is bound as GL_COLOR_ATTACHMENT0.

      if(!buffer.depthID)
        create3DDepthTexture(buffer.depthID, width, height, levels);

      if(!buffer.textureID)
        create2DDepthTexture(buffer.textureID, width, height);

      glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, buffer.depthID, 0, GLint(level));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.textureID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 3D texture to FBO as color but to store depth");
    }
    else
#endif
    {
      if(!buffer.depthID)
        create2DDepthTexture(buffer.depthID, width, height);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO to store depth");
    }

    if(hdr == HDR::Yes)
    {
      if(!buffer.normalsID)
        create2DColorTexture(buffer.normalsID, width, height, HDR::Yes, Alpha::No);

      if(!buffer.specularID)
        create2DColorTexture(buffer.specularID, width, height, HDR::Yes, Alpha::Yes);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer.normalsID, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer.specularID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D normal and specular textures to FBO");
      setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
    }
    else
      setDrawBuffers({GL_COLOR_ATTACHMENT0});

    if(printFBOError("Error Map::Private::prepareBuffer frame buffer not complete!"))
      return false;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(multisample == Multisample::Yes)
    {
      glEnable(GL_MULTISAMPLE);

      if(!buffer.multisampleFrameBuffer)
        glGenFramebuffers(1, &buffer.multisampleFrameBuffer);

      glBindFramebuffer(GL_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer second");

      if(!buffer.multisampleDepthRBO)
        createDepthRBO(buffer.multisampleDepthRBO, width, height);

      if(!buffer.multisampleTextureID)
        createMultisampleTexture(buffer.multisampleTextureID, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(!buffer.multisampleColorRBO)
        createColorRBO(buffer.multisampleColorRBO, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(hdr == HDR::Yes)
      {
        if(!buffer.multisampleNormalsTextureID)
          createMultisampleTexture(buffer.multisampleNormalsTextureID, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        if(!buffer.multisampleSpecularTextureID)
          createMultisampleTexture(buffer.multisampleSpecularTextureID, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        if(!buffer.multisampleNormalsRBO)
          createColorRBO(buffer.multisampleNormalsRBO, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        if(!buffer.multisampleSpecularRBO)
          createColorRBO(buffer.multisampleSpecularRBO, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      if(printFBOError("Error Map::Private::prepareBuffer multisample frame buffer not complete!"))
        return false;
    }
#endif

    glViewport(0, 0, TPGLsizei(width), TPGLsizei(height));

    glDepthMask(true);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DEBUG_printOpenGLError("prepareBuffer end");

    return true;
  }

  //################################################################################################
  //If we are rendering to a multisample FBO we need to blit the results to a non multisampled FBO
  //before we can actually use it. (thanks OpenGL)
  void swapMultisampledBuffer(FBO& buffer)
  {
#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(buffer.multisample == Multisample::Yes)
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.frameBuffer);
      DEBUG_printOpenGLError("swapMultisampledBuffer bind FBOs");

      glReadBuffer(GL_COLOR_ATTACHMENT0);
      DEBUG_printOpenGLError("swapMultisampledBuffer a");
      setDrawBuffers({GL_COLOR_ATTACHMENT0});
      DEBUG_printOpenGLError("swapMultisampledBuffer b");
      glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
      DEBUG_printOpenGLError("swapMultisampledBuffer blit color 0 and depth");

      if(buffer.hdr == HDR::Yes)
      {
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        setDrawBuffers({GL_COLOR_ATTACHMENT1});
        glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        DEBUG_printOpenGLError("swapMultisampledBuffer blit color 1");

        glReadBuffer(GL_COLOR_ATTACHMENT2);
        setDrawBuffers({GL_COLOR_ATTACHMENT2});
        glBlitFramebuffer(0, 0, GLint(buffer.width), GLint(buffer.height), 0, 0, GLint(buffer.width), GLint(buffer.height), GL_COLOR_BUFFER_BIT, GL_NEAREST);
        DEBUG_printOpenGLError("swapMultisampledBuffer blit color 2");

        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
      DEBUG_printOpenGLError("swapMultisampledBuffer bind FBO");
    }
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    TP_UNUSED(buffer);
#endif
  }

  //################################################################################################
  void deleteBuffer(FBO& buffer)
  {
    glBindTexture(GL_TEXTURE_2D, 0);

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

    if(buffer.normalsID)
    {
      glDeleteTextures(1, &buffer.normalsID);
      buffer.normalsID = 0;
    }

    if(buffer.specularID)
    {
      glDeleteTextures(1, &buffer.specularID);
      buffer.specularID = 0;
    }

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(buffer.multisampleFrameBuffer)
    {
      glDeleteFramebuffers(1, &buffer.multisampleFrameBuffer);
      buffer.multisampleFrameBuffer = 0;
    }

    if(buffer.multisampleTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleTextureID);
      buffer.multisampleTextureID = 0;
    }

    if(buffer.multisampleColorRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleColorRBO);
      buffer.multisampleColorRBO = 0;
    }

    if(buffer.multisampleDepthRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleDepthRBO);
      buffer.multisampleDepthRBO = 0;
    }

    if(buffer.multisampleNormalsTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleNormalsTextureID);
      buffer.multisampleNormalsTextureID = 0;
    }

    if(buffer.multisampleSpecularTextureID)
    {
      glDeleteTextures(1, &buffer.multisampleSpecularTextureID);
      buffer.multisampleSpecularTextureID = 0;
    }

    if(buffer.multisampleNormalsRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleNormalsRBO);
      buffer.multisampleNormalsRBO = 0;
    }

    if(buffer.multisampleSpecularRBO)
    {
      glDeleteRenderbuffers(1, &buffer.multisampleSpecularRBO);
      buffer.multisampleSpecularRBO = 0;
    }
#endif
  }

  //################################################################################################
  void invalidateBuffer(FBO& buffer)
  {
    buffer.frameBuffer = 0;
    buffer.textureID   = 0;
    buffer.depthID     = 0;
    buffer.normalsID   = 0;
    buffer.specularID   = 0;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    buffer.multisampleFrameBuffer = 0;
    buffer.multisampleTextureID   = 0;
    buffer.multisampleColorRBO    = 0;
    buffer.multisampleDepthRBO    = 0;

    buffer.multisampleNormalsTextureID  = 0;
    buffer.multisampleSpecularTextureID = 0;
    buffer.multisampleNormalsRBO        = 0;
    buffer.multisampleSpecularRBO       = 0;
#endif
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
  d->deleteShaders();

  delete d->controller;
  d->controller=nullptr;

  clearLayers();

  while(!d->fontRenderers.empty())
    delete tpTakeFirst(d->fontRenderers);

  for(size_t i=0; i<3; i++)
    d->deleteBuffer(d->intermediateFBOs[i]);

  d->deleteBuffer(d->pickingBuffer);
  d->deleteBuffer(d->renderToImageBuffer);

  for(auto& lightBuffer : d->lightBuffers)
    d->deleteBuffer(lightBuffer);

  if(d->spotLightTextureID)
  {
    deleteTexture(d->spotLightTextureID);
    d->spotLightTextureID = 0;
  }

  delete d->spotLightTexture;
  d->spotLightTexture=nullptr;

  d->preDeleteCalled = true;
}

//##################################################################################################
void Map::setOpenGLProfile(OpenGLProfile openGLProfile)
{
  d->openGLProfile = openGLProfile;
}

//##################################################################################################
void Map::setVisible(bool visible)
{
  d->visible = visible;
}

//##################################################################################################
OpenGLProfile Map::openGLProfile() const
{
  return d->openGLProfile;
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
bool Map::printFBOError(const std::string& description)
{
  if(auto e=glCheckFramebufferStatus(GL_FRAMEBUFFER); e != GL_FRAMEBUFFER_COMPLETE)
  {
    auto fboError=[](GLenum e)
    {
      switch(e)
      {
      case GL_FRAMEBUFFER_COMPLETE:                      return std::string("GL_FRAMEBUFFER_COMPLETE");
      case GL_FRAMEBUFFER_UNDEFINED:                     return std::string("GL_FRAMEBUFFER_UNDEFINED");
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         return std::string("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return std::string("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
      case GL_FRAMEBUFFER_UNSUPPORTED:                   return std::string("GL_FRAMEBUFFER_UNSUPPORTED");

#ifdef TP_ENABLE_MULTISAMPLE_FBO
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        return std::string("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER");
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        return std::string("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER");
      case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:      return std::string("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS");
      case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        return std::string("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
#endif
      }

      return std::to_string(int(e));
    };

    tpWarning() << description;
    tpWarning() << "FBO error: " << fboError(e);
    tpWarning() << glGetString(GL_VERSION);
    return true;
  }
  return false;
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

  for(auto l : d->layers)
    l->animate(timestampMS);
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
glm::vec3 Map::backgroundColor() const
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
void Map::setLights(const std::vector<tp_math_utils::Light>& lights)
{
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
void Map::setSpotLightTexture(Texture* spotLightTexture)
{
  delete d->spotLightTexture;
  d->spotLightTexture = spotLightTexture;
  if(d->spotLightTextureID)
  {
    makeCurrent();
    deleteTexture(d->spotLightTextureID);
    d->spotLightTextureID = 0;
  }
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
  layer->setMap(this, nullptr);
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
  d->updateSamplesRequired = true;
  d->maxSamples = maxSamples;
}

//##################################################################################################
size_t Map::maxSamples() const
{
  return d->maxSamples;
}

//##################################################################################################
void Map::setMaxSpotLightLevels(size_t maxSpotLightLevels)
{
  d->spotLightLevels = maxSpotLightLevels;
}

//##################################################################################################
size_t Map::maxSpotLightLevels() const
{
  return d->spotLightLevels;
}

//##################################################################################################
size_t Map::spotLightLevels() const
{
#ifdef TP_ENABLE_3D_TEXTURE
  return tpMin(tp_math_utils::Light::lightLevelOffsets().size(), d->spotLightLevels);
#else
  return 1;
#endif
}

//##################################################################################################
void Map::setShadowSamples(size_t shadowSamples)
{
  if(d->shadowSamples != shadowSamples)
  {
    d->shadowSamples = shadowSamples;
    d->deleteShaders();

    for(auto l : d->layers)
      l->lightsChanged(LightingModelChanged::No);
  }
}

//##################################################################################################
size_t Map::shadowSamples() const
{
  return d->shadowSamples;
}

//##################################################################################################
void Map::setHDR(HDR hdr)
{
  d->hdr = hdr;
}

//##################################################################################################
HDR Map::hdr() const
{
  return d->hdr;
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
  if(!d->prepareBuffer(d->renderToImageBuffer, d->width, d->height, CreateColorBuffer::Yes, Multisample::No, HDR::No, 1, 0))
    return nullptr;

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, 1.0f);

  //------------------------------------------------------------------------------------------------
  // Execute a picking render pass.
  d->renderInfo.resetPicking();
  d->renderInfo.pass = RenderPass::Picking;
  d->renderInfo.hdr = HDR::No;
  d->renderInfo.pickingType = pickingType;
  d->renderInfo.pos = pos;

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDepthMask(true);

  d->render();

  //------------------------------------------------------------------------------------------------
  // Read the small patch from around the picking position and then free up the frame buffers.

  //The size of the area to perform picking in, must be an odd number
  int windowX = tpBound(0, pos.x-left, width()-(pickingSize+1));
  int windowY = tpBound(0, (int(d->height)-pos.y)-left, int(height()-(pickingSize+1)));
  std::vector<unsigned char> pixels(pickingSize*pickingSize*4);

  switch(d->openGLProfile)
  {
  case OpenGLProfile::VERSION_100_ES:
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
  if(width<1 || height<1)
  {
    tpWarning() << "Error Map::renderToImage can't render to image smaller than 1 pixel.";
    return false;
  }

  auto originalWidth  = d->width;
  auto originalHeight = d->height;
  d->width = width;
  d->height = height;
  resizeGL(int(width), int(height));

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  DEBUG_printOpenGLError("renderToImage A");

  // Configure the frame buffer that the image will be rendered to.
  if(!d->prepareBuffer(d->renderToImageBuffer, width, height, CreateColorBuffer::Yes, Multisample::No, HDR::No, 1, 0))
  {
    tpWarning() << "Error Map::renderToImage failed to create render buffer.";
    return false;
  }

  DEBUG_printOpenGLError("renderToImage B");

  // Execute a render passes.
  paintGLNoMakeCurrent();
  DEBUG_printOpenGLError("renderToImage C1");

  // Swap the multisampled FBO into the non multisampled FBO.
  d->swapMultisampledBuffer(d->renderToImageBuffer);
  DEBUG_printOpenGLError("renderToImage C2");

  // Read the texture that we just generated
  glBindFramebuffer(GL_FRAMEBUFFER, d->renderToImageBuffer.frameBuffer);
  glReadPixels(0, 0, int(width), int(height), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
  DEBUG_printOpenGLError("renderToImage C3");

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
  d->width = originalWidth;
  d->height = originalHeight;
  resizeGL(int(originalWidth), int(originalHeight));
  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);
  glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));

  DEBUG_printOpenGLError("renderToImage D");

  return true;
}

//##################################################################################################
bool Map::renderToImage(size_t width, size_t height, tp_image_utils::ColorMapF& image, bool swapY)
{
  image.setSize(width, height);
  glm::vec4* pixels = image.data();

  if(width<1 || height<1)
  {
    tpWarning() << "Error Map::renderToImage can't render to image smaller than 1 pixel.";
    return false;
  }

  auto originalWidth  = d->width;
  auto originalHeight = d->height;
  resizeGL(int(width), int(height));

  GLint originalFrameBuffer = 0;
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);

  DEBUG_printOpenGLError("renderToImage A");

  // Configure the frame buffer that the image will be rendered to.
  if(!d->prepareBuffer(d->renderToImageBuffer, width, height, CreateColorBuffer::Yes, Multisample::No, HDR::Yes, 1, 0))
    return false;

  DEBUG_printOpenGLError("renderToImage B");

  // Execute a render passes.
  paintGLNoMakeCurrent();
  DEBUG_printOpenGLError("renderToImage C1");

  // Swap the multisampled FBO into the non multisampled FBO.
  d->swapMultisampledBuffer(d->renderToImageBuffer);
  DEBUG_printOpenGLError("renderToImage C2");

  // Read the texture that we just generated
  glReadPixels(0, 0, int(width), int(height), GL_RGBA, GL_FLOAT, pixels);
  DEBUG_printOpenGLError("renderToImage C3");

  if(swapY)
  {
    std::vector<glm::vec4> line{size_t(width)};
    glm::vec4* c = line.data();
    size_t rowLengthBytes = size_t(width)*sizeof(glm::vec4);
    size_t yMax = size_t(height)/2;
    for(size_t y=0; y<yMax; y++)
    {
      glm::vec4* a{pixels + y*size_t(width)};
      glm::vec4* b{pixels + (size_t(height-1)-y)*size_t(width)};

      memcpy(c, a, rowLengthBytes);
      memcpy(a, b, rowLengthBytes);
      memcpy(b, c, rowLengthBytes);
    }
  }

  // Return to the original viewport settings
  resizeGL(int(originalWidth), int(originalHeight));
  glBindFramebuffer(GL_FRAMEBUFFER, originalFrameBuffer);
  glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));

  DEBUG_printOpenGLError("renderToImage D");

  return true;
}

//##################################################################################################
void Map::deleteTexture(GLuint id)
{
  if(id>0)
    glDeleteTextures(1, &id);
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
const FBO& Map::currentReadFBO()
{
  return *d->currentReadFBO;
}

//##################################################################################################
const FBO& Map::currentDrawFBO()
{
  return *d->currentDrawFBO;
}

//##################################################################################################
const std::vector<FBO>& Map::lightBuffers() const
{
  return d->lightBuffers;
}

//##################################################################################################
GLuint Map::spotLightTexture() const
{
  if(!d->spotLightTexture || !d->spotLightTexture->imageReady())
    d->spotLightTexture = new DefaultSpritesTexture(d->q);

  if(!d->spotLightTextureID)
    d->spotLightTextureID = d->spotLightTexture->bindTexture();

  return d->spotLightTextureID;
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

#if defined(TP_MAPS_DEBUG) && !defined(TP_EMSCRIPTEN) && !defined(TP_OSX)
  {
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if(flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
      tpWarning() << "Got a debug context, registering debug callback.";
      glEnable(GL_DEBUG_OUTPUT);
      glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
      glDebugMessageCallback(tpOutputOpenGLDebug, nullptr);
      glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
    else
    {
      tpWarning() << "Failed to get debug GL context.";
    }
  }
#endif

  d->updateSamplesRequired = true;

  //Invalidate old state before initializing new state
  {
    for(auto& i : d->shaders)
    {
      i.second->invalidate();
      delete i.second;
    }
    d->shaders.clear();

    for(size_t i=0; i<3; i++)
      d->invalidateBuffer(d->intermediateFBOs[i]);

    d->invalidateBuffer(d->pickingBuffer);
    d->invalidateBuffer(d->renderToImageBuffer);

    for(auto& lightTexture : d->lightBuffers)
      d->invalidateBuffer(lightTexture);
    d->lightBuffers.clear();

    d->spotLightTextureID = 0;

    for(auto i : d->layers)
      i->invalidateBuffers();

    for(auto i : d->fontRenderers)
      i->invalidateBuffers();

    invalidateBuffersCallbacks();
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

#ifdef TP_ENABLE_MULTISAMPLE
  glEnable(GL_MULTISAMPLE);
#endif

  glClearColor(d->backgroundColor.x, d->backgroundColor.y, d->backgroundColor.z, 1.0f);
  d->initialized = true;

  d->controller->mapResized(int(d->width), int(d->height));
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
  DEBUG_printOpenGLError("paintGLNoMakeCurrent start");

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
      DEBUG_printOpenGLError("RenderPass::LightFBOs start");
#ifdef TP_FBO_SUPPORTED      
      d->renderInfo.hdr = HDR::No;

      while(d->lightBuffers.size() < d->lights.size())
        d->lightBuffers.emplace_back();

      while(d->lightBuffers.size() > d->lights.size())
      {
        auto buffer = tpTakeLast(d->lightBuffers);
        d->deleteBuffer(buffer);
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

        size_t levels=1;

#ifdef TP_ENABLE_3D_TEXTURE
        if(light.type == tp_math_utils::LightType::Spot)
          levels=d->spotLightLevels;
#endif

        lightBuffer.worldToTexture.resize(levels);
        for(size_t l=0; l<levels; l++)
        {
          if(!d->prepareBuffer(lightBuffer, d->lightTextureSize, d->lightTextureSize, CreateColorBuffer::No, Multisample::No, HDR::No, levels, l))
            return;

          d->controller->setCurrentLight(light, l);
          lightBuffer.worldToTexture[l] = d->controller->lightMatrices();
          d->render();
        }
      }
      DEBUG_printOpenGLError("RenderPass::LightFBOs prepare buffers");

      glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));
      glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
      DEBUG_printOpenGLError("RenderPass::LightFBOs bind default buffer");
#endif
      break;
    }

    case RenderPass::PrepareDrawFBO: //-------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::PrepareDrawFBO start");
#ifdef TP_FBO_SUPPORTED
      d->renderInfo.hdr = hdr();
      glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &originalFrameBuffer);
      d->currentDrawFBO = &d->intermediateFBOs[0];
      d->currentReadFBO = &d->intermediateFBOs[0];
      if(!d->prepareBuffer(*d->currentDrawFBO, d->width, d->height, CreateColorBuffer::Yes, Multisample::Yes, hdr(), 1, 0))
      {
        printOpenGLError("RenderPass::PrepareDrawFBO");
        return;
      }
#endif
      DEBUG_printOpenGLError("RenderPass::PrepareDrawFBO end");
      break;
    }

    case RenderPass::SwapDrawFBO: //----------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::SwapDrawFBO start");
#ifdef TP_FBO_SUPPORTED
      d->renderInfo.hdr = hdr();
      d->swapMultisampledBuffer(*d->currentDrawFBO);
      d->currentReadFBO = d->currentDrawFBO;
      d->currentDrawFBO = (d->currentReadFBO!=&d->intermediateFBOs[1])?&d->intermediateFBOs[1]:&d->intermediateFBOs[2];

      if(!d->prepareBuffer(*d->currentDrawFBO, d->width, d->height, CreateColorBuffer::Yes, Multisample::No, hdr(), 1, 0))
      {
        printOpenGLError("RenderPass::SwapDrawFBO");
        return;
      }
#endif
      DEBUG_printOpenGLError("RenderPass::SwapDrawFBO end");
      break;
    }

    case RenderPass::Background: //-----------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Background start");
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
      d->render();
      DEBUG_printOpenGLError("RenderPass::Background end");
      break;
    }

    case RenderPass::Normal: //---------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Normal start");
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(true);
      d->render();
      DEBUG_printOpenGLError("RenderPass::Normal end");
      break;
    }

    case RenderPass::Transparency: //---------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Transparency start");
      glEnable(GL_DEPTH_TEST);
      glDepthFunc(GL_LESS);
      glDepthMask(false);
      d->render();
      DEBUG_printOpenGLError("RenderPass::Transparency end");
      break;
    }

    case RenderPass::FinishDrawFBO: //--------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::FinishDrawFBO start");
#ifdef TP_FBO_SUPPORTED
      d->renderInfo.hdr = HDR::No;
      d->swapMultisampledBuffer(*d->currentDrawFBO);
      std::swap(d->currentDrawFBO, d->currentReadFBO);
      glBindFramebuffer(GL_FRAMEBUFFER, GLuint(originalFrameBuffer));
#endif
      DEBUG_printOpenGLError("RenderPass::FinishDrawFBO end");
      break;
    }

    case RenderPass::Text: //-----------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Text start");
      glDisable(GL_DEPTH_TEST);
      glDepthMask(false);
      d->render();
      DEBUG_printOpenGLError("RenderPass::Text end");
      break;
    }

    case RenderPass::GUI: //------------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::GUI start");
      glEnable(GL_SCISSOR_TEST);
      glDisable(GL_DEPTH_TEST);
      auto s = pixelScale();
      glScissor(0, 0, GLsizei(float(width())*s), GLsizei(float(height())*s));
      glDepthMask(false);
      d->render();
      glDisable(GL_SCISSOR_TEST);
      DEBUG_printOpenGLError("RenderPass::GUI end");
      break;
    }

    case RenderPass::Picking: //--------------------------------------------------------------------
    {
      tpWarning() << "Error: Performing a picking render pass in paintGL does not make sense.";
      break;
    }

    case RenderPass::Custom1: //--------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Custom1 start");
      if(d->custom1Start)
        d->custom1Start(d->renderInfo);

      d->render();

      if(d->custom1End)
        d->custom1End(d->renderInfo);

      DEBUG_printOpenGLError("RenderPass::Custom1 end");
      break;
    }

    case RenderPass::Custom2: //--------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Custom2 start");
      if(d->custom2Start)
        d->custom2Start(d->renderInfo);

      d->render();

      if(d->custom2End)
        d->custom2End(d->renderInfo);

      DEBUG_printOpenGLError("RenderPass::Custom2 end");
      break;
    }

    case RenderPass::Custom3: //--------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Custom3 start");
      if(d->custom3Start)
        d->custom3Start(d->renderInfo);

      d->render();

      if(d->custom3End)
        d->custom3End(d->renderInfo);

      DEBUG_printOpenGLError("RenderPass::Custom3 end");
      break;
    }

    case RenderPass::Custom4: //--------------------------------------------------------------------
    {
      DEBUG_printOpenGLError("RenderPass::Custom4 start");
      if(d->custom4Start)
        d->custom4Start(d->renderInfo);

      d->render();

      if(d->custom4End)
        d->custom4End(d->renderInfo);

      DEBUG_printOpenGLError("RenderPass::Custom4 end");
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

  d->width  = size_t(w);
  d->height = size_t(h);

  glViewport(0, 0, TPGLsizei(d->width), TPGLsizei(d->height));

  if(d->initialized)
    d->controller->mapResized(w, h);

  update();
}

//##################################################################################################
bool Map::mouseEvent(const MouseEvent& event)
{
  // If a layer or the controller has focus from a previous press event pass the release to it first.
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

    if(auto i = d->controller->m_hasMouseFocusFor.find(event.button); i!=d->controller->m_hasMouseFocusFor.end())
    {
      d->controller->m_hasMouseFocusFor.erase(i);
      if(d->controller->mouseEvent(event))
        return true;
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

  if(d->controller->mouseEvent(event))
  {
    if(event.type == MouseEventType::Press)
      d->controller->m_hasMouseFocusFor.insert(event.button);
    return true;
  }

  return false;
}

//##################################################################################################
bool Map::keyEvent(const KeyEvent& event)
{
  // If a layer or the controller has focus from a previous press event pass the release to it first.
  if(event.type == KeyEventType::Release)
  {
    for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
    {
      Layer* layer = (*(--l));
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

  for(Layer** l = d->layers.data() + d->layers.size(); l>d->layers.data();)
  {
    Layer* layer = (*(--l));
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

}
