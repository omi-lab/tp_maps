#ifndef tp_maps_OpenGL_h
#define tp_maps_OpenGL_h

#include "tp_maps/Globals.h"

#include <cstddef>

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
#  include <GLES2/gl2.h>

#elif defined(TP_GLES3) //--------------------------------------------------------------------------
#  include <GLES3/gl3.h>

#elif defined(TP_OSX) //----------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_GLSL_HEADERS_INCLUDED
#  include <gl3.h>
#  include <OpenGL/glext.h>
#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_410
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO

#elif defined(TP_IOS) //----------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_GLSL_HEADERS_INCLUDED
#  include <OpenGLES/ES3/gl.h>
#  define TP_GLES3

#elif defined(TP_EMSCRIPTEN) //---------------------------------------------------------------------
#  include <GLES3/gl3.h>
#  define TP_GLES3

#elif defined(TP_ANDROID) //------------------------------------------------------------------------
#  if __ANDROID_API__ < 18
#    include <GLES2/gl2.h>
#    define TP_GLES2
#  else
#    include <GLES3/gl3.h>
#    define TP_GLES3
#  endif
#elif defined(TP_WIN32) //--------------------------------------------------------------------------
#  include <GL/glew.h>
#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_130
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO

#elif defined(TP_LINUX)

#  define GL_GLEXT_PROTOTYPES

#  include <GLES3/gl32.h>
#  include <GL/gl.h>
#  include <GL/glext.h>

// Don't bring in qopenglext.h
#  ifndef __glext_h_
#    define __glext_h_ 1
#  endif

#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_460
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO


#else //--------------------------------------------------------------------------------------------
#  define GL_GLEXT_LEGACY
#  include <GLES3/gl3.h>
#  include <GL/gl.h>
#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_130
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE

#endif //-------------------------------------------------------------------------------------------

#ifdef TP_GL3 //------------------------------------------------------------------------------------
#  define TP_VERTEX_ARRAYS_SUPPORTED
#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GLSL_PICKING_SUPPORTED
#  define TP_FBO_SUPPORTED

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsizei = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;
#endif

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_100_ES

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT16
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
#  define TP_GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER

using TPGLsizei = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;
#endif

#ifdef TP_GLES3 //----------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::ShaderProfile::GLSL_300_ES

#  define TP_VERTEX_ARRAYS_SUPPORTED
//#  define tpGenVertexArrays glGenVertexArrays
//#  define tpBindVertexArray glBindVertexArray
//#  define tpDeleteVertexArrays glDeleteVertexArrays
//#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)
#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawElements(mode, count, type, indices)

//#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT16
//#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
//#  define TP_GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER
#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

#  define TP_GLSL_PICKING_SUPPORTED
#  define TP_FBO_SUPPORTED

using TPGLsizei = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;
#endif

namespace tp_maps
{

//##################################################################################################
struct OpenGLFBO
{
  tp_utils::StringID name;

  GLuint frameBuffer{0};

  GLuint textureID{0};  //!< The color buffer texture.
  GLuint depthID{0};    //!< The depth buffer texture.

  // These are used for deferred rendering, useful for SSR and post processing. Only created if
  // using ExtendedFBO::Yes.
  GLuint normalsID{0};  //!< The normals of each fragment, useful for ray marching.
  GLuint specularID{0}; //!< The specular colors of each fragment, as well as the shininess in the alpha.

  size_t width{1};
  size_t height{1};
  size_t samples{1};

#ifdef TP_ENABLE_MULTISAMPLE_FBO
  GLuint multisampleFrameBuffer{0};
  // GLuint multisampleTextureID{0};

  GLuint multisampleColorRBO{0};
  GLuint multisampleDepthRBO{0};

  // GLuint multisampleNormalsTextureID{0};
  // GLuint multisampleSpecularTextureID{0};

  GLuint multisampleNormalsRBO{0};
  GLuint multisampleSpecularRBO{0};
#endif

  Matrices worldToTexture; //!< For lighting this is used to map world coords onto the texture.

  Multisample multisample{Multisample::No};      //!< Yes if multisample buffers have been created.
  Multisample multisampleParam{Multisample::No}; //!< Yes if multisample buffers was requested.
  HDR hdr{HDR::No};                              //!< Yes if HDR buffers have been created.
  ExtendedFBO extendedFBO{ExtendedFBO::No};      //!< Yes if deferred rendering buffers have been created.

  bool blitRequired{false};
};

}

#endif
