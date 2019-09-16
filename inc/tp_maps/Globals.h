#ifndef tp_maps_Globals_h
#define tp_maps_Globals_h

#include "tp_utils/StringID.h"

#include <unordered_map>

#if defined(TP_MAPS_LIBRARY)
#  define TP_MAPS_SHARED_EXPORT TP_EXPORT
#else
#  define TP_MAPS_SHARED_EXPORT TP_IMPORT
#endif

#ifdef TP_GLES2_100 //------------------------------------------------------------------------------

#elif defined(TDP_OSX) //---------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  include <gl3.h>
#  include <OpenGL/glext.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_410

#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GLSL_PICKING

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsize = GLuint;
using TPGLfloat = float;
using TPGLenum = GLenum;

#elif defined(TDP_IOS) //---------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  include <GLES2/gl2.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_300_ES

#  define tpGenVertexArrays glGenVertexArraysOES
#  define tpBindVertexArray glBindVertexArrayOES
#  define tpDeleteVertexArrays glDeleteVertexArraysOES
#  define tpDrawElements(mode, count, type, indices) glDrawElements(mode, count, type, indices)

#  define TP_GLSL_PICKING

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32_OES
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsize = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;

#elif defined(TDP_EMSCRIPTEN) //--------------------------------------------------------------------
#  include <GLES3/gl3.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_100_ES

#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT16
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
#  define TP_GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER

using TPGLsize = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;

#elif defined(TDP_ANDROID) //-----------------------------------------------------------------------
#  define TP_GLES2_100

#elif defined(TDP_WIN32)

#include <GL/glew.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130

#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GLSL_PICKING

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsize = GLuint;
using TPGLfloat = float;
using TPGLenum = GLenum;
#else //--------------------------------------------------------------------------------------------
#  include <GLES3/gl3.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130

#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GLSL_PICKING

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsize  = GLuint;
using TPGLfloat = float;
using TPGLenum  = GLint;

#endif //-------------------------------------------------------------------------------------------


#ifdef TP_GLES2_100 //------------------------------------------------------------------------------
#  include <GLES3/gl2.h>

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_100_ES

#  define tpGenVertexArrays glGenVertexArrays
#  define tpBindVertexArray glBindVertexArray
#  define tpDeleteVertexArrays glDeleteVertexArrays
#  define tpDrawElements(mode, count, type, indices) glDrawRangeElements(mode, 0, count, GLsizei(count), type, indices)

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT32F
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT24
#  define TP_GL_DRAW_FRAMEBUFFER GL_DRAW_FRAMEBUFFER

using TPGLsize = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;
#endif

//##################################################################################################
//! A simple 3D engine
namespace tp_maps
{
TDP_DECLARE_ID(                      defaultSID,                          "Default");
TDP_DECLARE_ID(                       screenSID,                           "Screen");
TDP_DECLARE_ID(                   lineShaderSID,                      "Line shader");
TDP_DECLARE_ID(                  imageShaderSID,                     "Image shader");
TDP_DECLARE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TDP_DECLARE_ID(               materialShaderSID,                  "Material shader");
TDP_DECLARE_ID(               yuvImageShaderSID,                 "YUV image shader");
TDP_DECLARE_ID(                   fontShaderSID,                      "Font shader");
TDP_DECLARE_ID(                  frameShaderSID,                     "Frame shader");

//##################################################################################################
enum class RenderPass
{
  Background,
  Normal,
  Transparency,
  Reflection,
  Text,
  GUI,
  Picking,
  Custom1,
  Custom2,
  Custom3,
  Custom4
};

//##################################################################################################
enum class OpenGLProfile
{
  VERSION_110,  // Not really supported by most of the shaders in tp_maps
  VERSION_120,
  VERSION_130,
  VERSION_140,
  VERSION_150,
  VERSION_330,
  VERSION_400,
  VERSION_410,
  VERSION_420,
  VERSION_430,
  VERSION_440,
  VERSION_450,
  VERSION_460,
  VERSION_100_ES,
  VERSION_300_ES,
  VERSION_310_ES,
  VERSION_320_ES
};

//##################################################################################################
struct ShaderString
{
  TP_NONCOPYABLE(ShaderString);
  ShaderString(const char* text);
  const char* data(OpenGLProfile openGLProfile);

private:
  const std::string m_str;
  std::unordered_map<OpenGLProfile, std::string> m_parsed;
};

//##################################################################################################
struct ShaderResource
{
  TP_NONCOPYABLE(ShaderResource);
  ShaderResource(const std::string& resourceName);
  const char* data(OpenGLProfile openGLProfile);

private:
  const std::string m_resourceName;
  std::unordered_map<OpenGLProfile, std::string> m_parsed;
};
}

#endif
