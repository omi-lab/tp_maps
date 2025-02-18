#include "tp_maps/subsystems/open_gl/OpenGLBuffers.h"
#ifdef TP_MAPS_SUBSYSTEM_OPENGL

#include "tp_maps/Errors.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#ifdef TP_MAPS_DEBUG
#  define DEBUG_printOpenGLError(A) Errors::printOpenGLError(A)
#else
#  define DEBUG_printOpenGLError(A) do{}while(false)
#endif

namespace tp_maps
{

//##################################################################################################
// enum class SamplesUpdate { None, Settings, Heuristic };

//##################################################################################################
struct OpenGLBuffers::Private
{
  Map* map;

  bool   samplesNeedUpdate{false};
  size_t maxSamples{1};
  size_t samples{1};

  std::unordered_map<tp_utils::StringID, OpenGLFBO*> storedBuffers;

  //################################################################################################
  Private(Map* map_):
    map(map_)
  {

  }

  //################################################################################################
  size_t glMaxSamples()
  {
    static size_t glMaxSamples_{0};

    if (glMaxSamples_ == 0)
    {
      int max=0;
      glGetIntegerv(GL_MAX_SAMPLES, &max);
      glMaxSamples_ = static_cast<size_t>(tpBound(1, max, 32));
    }

    return glMaxSamples_;
  }

  //################################################################################################
  void updateSamples()
  {
    // Update is not required
    if (!samplesNeedUpdate)
      return;
    samplesNeedUpdate = false;

#ifdef TP_GLES3
    const bool fromSettings = false;
#else
    const bool fromSettings = true;
#endif

    // Validate that sample value is in correct range
    static auto const inBounds = [this](size_t v) { return tpBound(size_t(1), v, size_t(32)); };

    // Update based on user settings or heuristics
    const size_t glMax = glMaxSamples();

    if (fromSettings)
      samples = inBounds(tpMin(maxSamples, glMax));
    else
      samples = maxSamples = inBounds(glMax/2);  // Watchout: Modifying maxSamples too!

    tpWarning() << "Samples set to: " << samples
                << " / Based on: " << (fromSettings ? "Settings" : "Heuristics")
                << " / glMaxSamples: " << glMax;
  }

  //################################################################################################
  GLint colorFormatF(Alpha alpha)
  {
#ifdef TP_GLES2
    return (alpha==Alpha::Yes)?GL_RGBA32F_EXT:GL_RGB16F_EXT;
#else
    switch(map->shaderProfile())
    {
      case ShaderProfile::GLSL_100_ES: [[fallthrough]];
      case ShaderProfile::GLSL_300_ES: [[fallthrough]];
      case ShaderProfile::GLSL_310_ES: [[fallthrough]];
      case ShaderProfile::GLSL_320_ES:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB16F;
      default:
      return (alpha==Alpha::Yes)?GL_RGBA32F:GL_RGB32F;
    }
#endif
  }

  //################################################################################################
  struct DepthFormatData
  {
    GLint  internalFormat = TP_GL_DEPTH_COMPONENT24;
    GLenum type           = GL_UNSIGNED_SHORT;
  };
  DepthFormatData depthFormat()
  {
    DepthFormatData dfd {};

    switch(map->shaderProfile())
    {
      case ShaderProfile::GLSL_100_ES:
        dfd.internalFormat = GL_DEPTH_COMPONENT;
        dfd.type = GL_UNSIGNED_SHORT;
        break;

      case ShaderProfile::GLSL_300_ES: [[fallthrough]];
      case ShaderProfile::GLSL_310_ES: [[fallthrough]];
      case ShaderProfile::GLSL_320_ES:
        dfd.internalFormat = TP_GL_DEPTH_COMPONENT32;
        dfd.type = GL_FLOAT;
        break;

      default:
        dfd.internalFormat = TP_GL_DEPTH_COMPONENT24;
        dfd.type = GL_UNSIGNED_SHORT;
        break;
    }

    return dfd;
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
      // On ES force alpha as GL_RGB32F is not an option but GL_RGBA32F is.
      if(alpha == Alpha::No)
      {
        switch (map->shaderProfile())
        {
          case ShaderProfile::GLSL_100_ES: [[fallthrough]];
          case ShaderProfile::GLSL_300_ES: [[fallthrough]];
          case ShaderProfile::GLSL_310_ES: [[fallthrough]];
          case ShaderProfile::GLSL_320_ES:
          alpha = Alpha::Yes;
          break;

          default:
          break;
        }
      }

      if(alpha == Alpha::No)
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGB, GL_FLOAT, nullptr);
      else
        glTexImage2D(GL_TEXTURE_2D, 0, colorFormatF(alpha), TPGLsizei(width), TPGLsizei(height), 0, GL_RGBA, GL_FLOAT, nullptr);
      DEBUG_printOpenGLError("create2DColorTexture C");
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    DEBUG_printOpenGLError("create2DColorTexture generate 2D texture for color buffer");
  }

  //################################################################################################
  void create2DDepthTexture(GLuint& depthID, size_t width, size_t height)
  {
    glGenTextures(1, &depthID);

    glBindTexture(GL_TEXTURE_2D, depthID);

    auto const [iFormat, type] = depthFormat();
    glTexImage2D(GL_TEXTURE_2D, 0, iFormat, TPGLsizei(width), TPGLsizei(height), 0, GL_DEPTH_COMPONENT, type, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    DEBUG_printOpenGLError("create2DDepthTexture generate 2D texture for depth buffer");
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

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    DEBUG_printOpenGLError("createColorRBO multisample RBO for attachment " + std::to_string(attachment));
  }

  //################################################################################################
  void createDepthRBO(GLuint& rboID, size_t width, size_t height)
  {
    auto const [iFormat, _] = depthFormat();

    glGenRenderbuffers(1, &rboID);
    glBindRenderbuffer(GL_RENDERBUFFER, rboID);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, TPGLsizei(samples), iFormat, TPGLsizei(width), TPGLsizei(height));
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboID);
    DEBUG_printOpenGLError("createDepthRBO multisample RBO for depth");
  }

  //################################################################################################
  void setDrawBuffers(const std::vector<GLenum>& buffers)
  {
    switch(map->shaderProfile())
    {
      default:
      glDrawBuffers(TPGLsizei(buffers.size()), buffers.data());
      break;

      case ShaderProfile::GLSL_100_ES:
      break;
    }

  }

  //################################################################################################
  void blitAttachment(OpenGLFBO const & buffer, GLint attachmentIndex=0, bool depthBit=false)
  {
    GLint  const w = GLint(buffer.width);
    GLint  const h = GLint(buffer.height);
    GLenum const attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;

    GLint mask = GL_COLOR_BUFFER_BIT;
    if (depthBit) mask |= GL_DEPTH_BUFFER_BIT;

    glReadBuffer(attachment);
    DEBUG_printOpenGLError("swapMultisampledBuffer a");

    glDrawBuffers(1, &attachment);
    DEBUG_printOpenGLError("swapMultisampledBuffer b");

    glBlitFramebuffer(0,0,w,h, 0,0,w,h, mask, GL_NEAREST);
    DEBUG_printOpenGLError("swapMultisampledBuffer blit color [and depth]");
  }

  //################################################################################################
  //! Create and bind an FBO.
  /*!
  This will delete and recreate the buffer if required.

  The rules for buffer formats are quite complicated and vary between versions.
  Section: 4.4.4
  https://www.khronos.org/registry/OpenGL/specs/es/3.0/es_spec_3.0.pdf

  \param buffer holds the resource ID's generated by this function.
  \param width of the buffer pixels.
  \param height of the buffer in pixels.
  \param createColorBuffer should be true if we need to create a color buffer, false for shadows.
  \param multisample should be true to enable antialiasing (MSAA).
  \param hdr are we using 8 bit or HDR buffers.
  \param extendedFBO are we creating extra buffers for normals and specular.

  \return true if we managed to create a functional FBO.
  */
  bool  prepareBuffer(OpenGLFBO& buffer,
                     size_t width,
                     size_t height,
                     CreateColorBuffer createColorBuffer,
                     Multisample multisample,
                     HDR hdr,
                     ExtendedFBO extendedFBO,
                     bool clear)
  {
    DEBUG_printOpenGLError("prepareBuffer Start");

    assert(buffer.name.isValid());

    buffer.blitRequired = false;
    buffer.multisampleParam = multisample;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if (multisample==Multisample::Yes) {
      // tpDebug() << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
      updateSamples();
    }
#endif

    multisample = (multisample==Multisample::Yes && (samples>1))?Multisample::Yes:Multisample::No;

    if(buffer.width!=width || buffer.height!=height || buffer.samples!=samples || buffer.hdr != hdr || buffer.extendedFBO != extendedFBO || buffer.multisample != multisample)
      deleteBuffer(buffer);

    if(!buffer.frameBuffer)
    {
      glGenFramebuffers(1, &buffer.frameBuffer);
      buffer.width       = width;
      buffer.height      = height;
      buffer.samples     = samples;
      buffer.hdr         = hdr;
      buffer.extendedFBO = extendedFBO;
      buffer.multisample = multisample;
    }

    DEBUG_printOpenGLError("prepareBuffer Init");

    glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
    DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer first");

    // Some versions of OpenGL must have a color buffer even if we are not going to use it.
    if(map->shaderProfile() == ShaderProfile::GLSL_100_ES)
      createColorBuffer = CreateColorBuffer::Yes;

    if(createColorBuffer == CreateColorBuffer::Yes)
    {
      if(!buffer.textureID)
        create2DColorTexture(buffer.textureID, width, height, hdr, Alpha::No);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.textureID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO");
    }

    {
      if(!buffer.depthID)
        create2DDepthTexture(buffer.depthID, width, height);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer.depthID, 0);
      DEBUG_printOpenGLError("prepareBuffer bind 2D texture to FBO to store depth");
    }

    if(extendedFBO == ExtendedFBO::Yes)
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

    if(Errors::printFBOError(buffer, "Error Map::Private::prepareBuffer frame buffer not complete!")){
      // deleteBuffer(buffer);
      return false;
    }

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(multisample == Multisample::Yes)
    {
      buffer.blitRequired = true;
// #if !defined(TP_GLES3)
//       glEnable(GL_MULTISAMPLE);
// #endif

      if(!buffer.multisampleFrameBuffer)
        glGenFramebuffers(1, &buffer.multisampleFrameBuffer);

      glBindFramebuffer(GL_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      DEBUG_printOpenGLError("prepareBuffer glBindFramebuffer second");

      if(!buffer.multisampleDepthRBO)
        createDepthRBO(buffer.multisampleDepthRBO, width, height);

      // if(!buffer.multisampleTextureID)
      //   createMultisampleTexture(buffer.multisampleTextureID, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(!buffer.multisampleColorRBO)
        createColorRBO(buffer.multisampleColorRBO, width, height, hdr, Alpha::No, GL_COLOR_ATTACHMENT0);

      if(extendedFBO == ExtendedFBO::Yes)
      {
        // if(!buffer.multisampleNormalsTextureID)
        //   createMultisampleTexture(buffer.multisampleNormalsTextureID, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        // if(!buffer.multisampleSpecularTextureID)
        //   createMultisampleTexture(buffer.multisampleSpecularTextureID, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        if(!buffer.multisampleNormalsRBO)
          createColorRBO(buffer.multisampleNormalsRBO, width, height, HDR::Yes, Alpha::No, GL_COLOR_ATTACHMENT1);

        if(!buffer.multisampleSpecularRBO)
          createColorRBO(buffer.multisampleSpecularRBO, width, height, HDR::Yes, Alpha::Yes, GL_COLOR_ATTACHMENT2);

        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      if(Errors::printFBOError(buffer, "Error Map::Private::prepareBuffer multisample frame buffer not complete!"))
        return false;
    }
#endif

    glViewport(0, 0, TPGLsizei(width), TPGLsizei(height));

    glDepthMask(true);

    if(clear)
    {
      glClearDepthf(1.0f);

      auto backgroundColor = map->backgroundColor();
      glClearColor(backgroundColor.x, backgroundColor.y, backgroundColor.z, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    DEBUG_printOpenGLError("prepareBuffer end");

    storedBuffers[buffer.name] = &buffer;

    return true;
  }

  //################################################################################################
  //If we are rendering to a multisample FBO we need to blit the results to a non multisampled FBO
  //before we can actually use it. (thanks OpenGL)
  void swapMultisampledBuffer(OpenGLFBO& buffer, bool bindNormalFBO)
  {
    if (!buffer.blitRequired)
      return;

    buffer.blitRequired = false;


#ifdef TP_ENABLE_MULTISAMPLE_FBO
    if(buffer.multisample == Multisample::Yes)
    {
      // Track stack state :: This must be before any binding !
      int prevDrawFboID = -1;
      glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &prevDrawFboID);

      glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer.multisampleFrameBuffer);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, buffer.frameBuffer);
      DEBUG_printOpenGLError("swapMultisampledBuffer bind FBOs");

      blitAttachment(buffer, 0, true);

      if(buffer.extendedFBO == ExtendedFBO::Yes)
      {
        blitAttachment(buffer, 1);
        blitAttachment(buffer, 2);
        setDrawBuffers({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2});
      }
      else
        setDrawBuffers({GL_COLOR_ATTACHMENT0});

      if (!bindNormalFBO && prevDrawFboID != buffer.multisampleFrameBuffer)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, prevDrawFboID);
        DEBUG_printOpenGLError("swapMultisampledBuffer bind Prev FBO");
      }
      else
      {
        glBindFramebuffer(GL_FRAMEBUFFER, buffer.frameBuffer);
        DEBUG_printOpenGLError("swapMultisampledBuffer bind Normal FBO");
      }
    }
#else
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    TP_UNUSED(buffer);
#endif
  }

  //################################################################################################
  void deleteBuffer(OpenGLFBO& buffer)
  {
    for(auto i=storedBuffers.begin(); i!=storedBuffers.end(); ++i)
    {
      if(i->second == &buffer)
      {
        storedBuffers.erase(i);
        break;
      }
    }

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

    // if(buffer.multisampleTextureID)
    // {
    //   glDeleteTextures(1, &buffer.multisampleTextureID);
    //   buffer.multisampleTextureID = 0;
    // }

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

    // if(buffer.multisampleNormalsTextureID)
    // {
    //   glDeleteTextures(1, &buffer.multisampleNormalsTextureID);
    //   buffer.multisampleNormalsTextureID = 0;
    // }

    // if(buffer.multisampleSpecularTextureID)
    // {
    //   glDeleteTextures(1, &buffer.multisampleSpecularTextureID);
    //   buffer.multisampleSpecularTextureID = 0;
    // }

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
  void invalidateBuffer(OpenGLFBO& buffer)
  {
    buffer.frameBuffer = 0;
    buffer.textureID   = 0;
    buffer.depthID     = 0;
    buffer.normalsID   = 0;
    buffer.specularID  = 0;

#ifdef TP_ENABLE_MULTISAMPLE_FBO
    buffer.multisampleFrameBuffer = 0;
    // buffer.multisampleTextureID   = 0;
    buffer.multisampleColorRBO    = 0;
    buffer.multisampleDepthRBO    = 0;

    // buffer.multisampleNormalsTextureID  = 0;
    // buffer.multisampleSpecularTextureID = 0;
    buffer.multisampleNormalsRBO        = 0;
    buffer.multisampleSpecularRBO       = 0;
#endif
  }
};

//##################################################################################################
OpenGLBuffers::OpenGLBuffers(Map* map):
  d(new Private(map))
{

}

//##################################################################################################
OpenGLBuffers::~OpenGLBuffers()
{
  delete d;
}

//##################################################################################################
bool OpenGLBuffers::prepareBuffer(OpenGLFBO& buffer,
                                  size_t width,
                                  size_t height,
                                  CreateColorBuffer createColorBuffer,
                                  Multisample multisample,
                                  HDR hdr,
                                  ExtendedFBO extendedFBO,
                                  bool clear) const
{
  return d->prepareBuffer(buffer,
                          width,
                          height,
                          createColorBuffer,
                          multisample,
                          hdr,
                          extendedFBO,
                          clear);
}

//##################################################################################################
bool OpenGLBuffers::bindBuffer(OpenGLFBO& buffer) const
{
  return d->prepareBuffer(buffer,
                          buffer.width,
                          buffer.height,
                          buffer.textureID!=0?CreateColorBuffer::Yes:CreateColorBuffer::No,
                          buffer.multisample,
                          buffer.hdr,
                          buffer.extendedFBO,
                          false);
}

//##################################################################################################
void OpenGLBuffers::invalidateBuffer(OpenGLFBO& fbo) const
{
  d->invalidateBuffer(fbo);
}

//##################################################################################################
void OpenGLBuffers::deleteBuffer(OpenGLFBO& fbo) const
{
  d->deleteBuffer(fbo);
}

//##################################################################################################
void OpenGLBuffers::swapMultisampledBuffer(OpenGLFBO& fbo, bool bindNormalFBO) const
{
  d->swapMultisampledBuffer(fbo, bindNormalFBO);
}

//##################################################################################################
void OpenGLBuffers::setDrawBuffers(const std::vector<GLenum>& buffers) const
{
  d->setDrawBuffers(buffers);
}

//##################################################################################################
void OpenGLBuffers::setMaxSamples(size_t maxSamples)
{
  d->samplesNeedUpdate = true;
  d->maxSamples = tpMax(size_t(1), maxSamples);
}

//##################################################################################################
size_t OpenGLBuffers::maxSamples() const
{
  return d->maxSamples;
}

//##################################################################################################
void OpenGLBuffers::initializeGL()
{
  d->samplesNeedUpdate = true;
}

//##################################################################################################
const std::unordered_map<tp_utils::StringID, OpenGLFBO*>& OpenGLBuffers::storedBuffers() const
{
  return d->storedBuffers;
}

}
#endif
