#include "tp_maps/Errors.h"
#include "tp_maps/Map.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include "tp_utils/DebugUtils.h"
#include "tp_utils/StackTrace.h" // IWYU pragma: keep

#ifdef TP_MAPS_DEBUG
#  define DEBUG_printOpenGLError(A) Errors::printOpenGLError(A)
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
struct Errors::Private
{
  Map* map;

  bool updateSamplesRequired{true};
  size_t maxSamples{1};
  size_t samples{1};

  //################################################################################################
  Private(Map* map_):
    map(map_)
  {

  }
};

//##################################################################################################
Errors::Errors(Map* map):
  d(new Private(map))
{

}

//##################################################################################################
Errors::~Errors()
{
  delete d;
}

//##################################################################################################
void Errors::initializeGL()
{
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
}

//##################################################################################################
void Errors::printOpenGLError(const std::string& description)
{
  if(GLenum error = glGetError(); error != GL_NO_ERROR)
    printOpenGLError(description, error);
}

//##################################################################################################
void Errors::printOpenGLError(const std::string& description, GLenum error)
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

//##################################################################################################
bool Errors::printFBOError(OpenGLFBO& buffer, const std::string& description)
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

    tpWarning() << "FBO details:\n" <<
                   "  frameBuffer:                  " << buffer.frameBuffer << '\n' <<
                   "  textureID:                    " << buffer.textureID   << '\n' <<
                   "  depthID:                      " << buffer.depthID     << '\n' <<
                   "  normalsID:                    " << buffer.normalsID   << '\n' <<
                   "  specularID:                   " << buffer.specularID  << '\n' <<
                   "  width:                        " << buffer.width       << '\n' <<
                   "  height:                       " << buffer.height      << '\n' <<
                   "  samples:                      " << buffer.samples     << '\n' <<
               #ifdef TP_ENABLE_MULTISAMPLE_FBO
                   "  multisampleFrameBuffer:       " << buffer.multisampleFrameBuffer       << '\n' <<
                   // "  multisampleTextureID:         " << buffer.multisampleTextureID         << '\n' <<
                   "  multisampleColorRBO:          " << buffer.multisampleColorRBO          << '\n' <<
                   "  multisampleDepthRBO:          " << buffer.multisampleDepthRBO          << '\n' <<
                   // "  multisampleNormalsTextureID:  " << buffer.multisampleNormalsTextureID  << '\n' <<
                   // "  multisampleSpecularTextureID: " << buffer.multisampleSpecularTextureID << '\n' <<
                   "  multisampleNormalsRBO:        " << buffer.multisampleNormalsRBO        << '\n' <<
                   "  multisampleSpecularRBO:       " << buffer.multisampleSpecularRBO       << '\n' <<
               #endif
                   "  multisample:                  " << (buffer.multisample==Multisample::Yes?"Yes":"No")  << '\n' <<
                   "  hdr:                          " << (buffer.hdr==HDR::Yes?"Yes":"No")  << '\n' <<
                   "  extendedFBO:                  " << (buffer.extendedFBO==ExtendedFBO::Yes?"Yes":"No");

    return true;
  }
  return false;
}

}
