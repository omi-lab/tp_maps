#ifndef tp_maps_Globals_h
#define tp_maps_Globals_h

#include "tp_utils/StringID.h"

#include <unordered_map>

#if defined(TP_MAPS_LIBRARY)
#  define TP_MAPS_SHARED_EXPORT TP_EXPORT
#else
#  define TP_MAPS_SHARED_EXPORT TP_IMPORT
#endif

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
#  include <GLES2/gl2.h>

#elif defined(TP_GLES3) //--------------------------------------------------------------------------
#  include <GLES3/gl3.h>

#elif defined(TP_OSX) //----------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  include <gl3.h>
#  include <OpenGL/glext.h>
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_410
#  define TP_GL3

#elif defined(TP_IOS) //----------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#  include <OpenGLES/ES3/gl.h>
#  define TP_GLES3

#elif defined(TP_EMSCRIPTEN) //---------------------------------------------------------------------
#  define TP_GLES3

#elif defined(TP_ANDROID) //------------------------------------------------------------------------
#  if __ANDROID_API__ < 18
#    include <GLES2/gl2.h>
#    define TP_GLES2
#  else
#    define TP_GLES3
#  endif

#elif defined(TP_WIN32) //--------------------------------------------------------------------------
#include <GL/glew.h>
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130
#  define TP_GL3

#else //--------------------------------------------------------------------------------------------
#  include <GLES3/gl3.h>
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130
#  define TP_GL3

#endif //-------------------------------------------------------------------------------------------

#ifdef TP_GL3 //------------------------------------------------------------------------------------
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
//using TPGLenum  = GLint;
#endif

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
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

#if TP_GLES3 //-------------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_300_ES

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

#  define TP_GLSL_PICKING

using TPGLsize = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;

#endif

//##################################################################################################
//! A simple 3D engine.
namespace tp_maps
{
TP_DECLARE_ID(                      defaultSID,                          "Default");
TP_DECLARE_ID(                       screenSID,                           "Screen");
TP_DECLARE_ID(                   lineShaderSID,                      "Line shader");
TP_DECLARE_ID(                  imageShaderSID,                     "Image shader");
TP_DECLARE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DECLARE_ID(               materialShaderSID,                  "Material shader");
TP_DECLARE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DECLARE_ID(                   fontShaderSID,                      "Font shader");
TP_DECLARE_ID(                  frameShaderSID,                     "Frame shader");

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
/*
GLSL Version 	OpenGL Version 	Date               Shader Preprocessor
1.10.59[1]    2.0             30 April     2004  #version 110
1.20.8[2]     2.1             07 September 2006  #version 120
1.30.10[3]    3.0             22 November  2009  #version 130
1.40.08[4]    3.1             22 November  2009  #version 140
1.50.11[5]    3.2             04 December  2009  #version 150
3.30.6[6]     3.3             11 March     2010  #version 330
4.00.9[7]     4.0             24 July      2010  #version 400
4.10.6[8]     4.1             24 July      2010  #version 410
4.20.11[9]    4.2             12 December  2011  #version 420
4.30.8[10]    4.3             7 February   2013  #version 430
4.40.9[11]    4.4             16 June      2014  #version 440
4.50.7[12]    4.5             09 May       2017  #version 450
4.60.5[13]    4.6             14 June      2018  #version 460
*/
enum class OpenGLProfile
{
  VERSION_110 = 20,  // Not really supported by most of the shaders in tp_maps
  VERSION_120 = 21,
  VERSION_130 = 30,
  VERSION_140 = 31,
  VERSION_150 = 32,
  VERSION_330 = 33,
  VERSION_400 = 40,
  VERSION_410 = 41,
  VERSION_420 = 42,
  VERSION_430 = 43,
  VERSION_440 = 44,
  VERSION_450 = 45,
  VERSION_460 = 46,
  VERSION_100_ES,
  VERSION_300_ES,
  VERSION_310_ES,
  VERSION_320_ES
};

//##################################################################################################
enum class OpenGLDepth
{
  OFF=0,
  ON_16=16,
  ON_24=24,
  ON_32
};

//##################################################################################################
struct OpenGLConfig
{
  OpenGLProfile profile{TP_DEFAULT_PROFILE};
  OpenGLDepth depth{OpenGLDepth::ON_24};
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
