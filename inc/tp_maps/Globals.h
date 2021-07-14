#ifndef tp_maps_Globals_h
#define tp_maps_Globals_h

#include "tp_utils/StringID.h"

#include "tp_math_utils/Geometry3D.h"
#include "tp_math_utils/Light.h"

#include "glm/glm.hpp"

#include "json.hpp"

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
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO
#  define TP_ENABLE_3D_TEXTURE

#elif defined(TP_IOS) //----------------------------------------------------------------------------
#  define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
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
#include <GL/glew.h>
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO
#  define TP_ENABLE_3D_TEXTURE

#elif defined(TP_LINUX)

#  define GL_GLEXT_PROTOTYPES

#  include <GLES3/gl32.h>
#  include <GL/gl.h>
#  include <GL/glext.h>

// Don't bring in qopenglext.h
#  ifndef __glext_h_
#    define __glext_h_ 1
#  endif

#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130
#  define TP_GL3
#  define TP_ENABLE_MULTISAMPLE
#  define TP_ENABLE_MULTISAMPLE_FBO
#  define TP_ENABLE_3D_TEXTURE


#else //--------------------------------------------------------------------------------------------
#  define GL_GLEXT_LEGACY
#  include <GLES3/gl3.h>
#  include <GL/gl.h>
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_130
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
//using TPGLenum  = GLint;
#endif

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_100_ES

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT16
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
#  define TP_GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER

using TPGLsizei = GLsizei;
using TPGLfloat = float;
using TPGLenum = GLenum;
#endif

#ifdef TP_GLES3 //-------------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_300_ES

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

#define      TP_UP_KEY         82
#define      TP_LEFT_KEY       80
#define      TP_RIGHT_KEY      79
#define      TP_DOWN_KEY       81

#define      TP_W_KEY          26
#define      TP_A_KEY           4
#define      TP_S_KEY          22
#define      TP_D_KEY           7

#define      TP_SPACE_KEY      44

#define      TP_L_SHIFT_KEY   229
#define      TP_R_SHIFT_KEY   225

#define      TP_L_CTRL_KEY    224

#define      TP_PAGE_UP_KEY    75
#define      TP_PAGE_DOWN_KEY  78

//##################################################################################################
//! A simple 3D engine.
namespace tp_maps
{
TP_DECLARE_ID(                      defaultSID,                          "Default");
TP_DECLARE_ID(                       screenSID,                           "Screen");
TP_DECLARE_ID(                   gizmoLayerSID,                      "Gizmo layer");
TP_DECLARE_ID(                   lineShaderSID,                      "Line shader");
TP_DECLARE_ID(                  imageShaderSID,                     "Image shader");
TP_DECLARE_ID(                image3DShaderSID,                  "Image 3D shader");
TP_DECLARE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DECLARE_ID(               materialShaderSID,                  "Material shader");
TP_DECLARE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DECLARE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DECLARE_ID(           depthImage3DShaderSID,            "Depth image 3D shader");
TP_DECLARE_ID(                   fontShaderSID,                      "Font shader");
TP_DECLARE_ID(                  frameShaderSID,                     "Frame shader");
TP_DECLARE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DECLARE_ID(                postSSRShaderSID,                  "Post ssr shader");
TP_DECLARE_ID(               postBlitShaderSID,                 "Post blit shader");
TP_DECLARE_ID(              postGammaShaderSID,                "Post gamma shader");
TP_DECLARE_ID(             backgroundShaderSID,                "Background shader");

//##################################################################################################
int staticInit();

//##################################################################################################
enum class RenderPass
{
  LightFBOs,     //!< Render depth maps from the point of view of lights to FBOs.
  PrepareDrawFBO,//!< Prepare the initial draw FBO ready for drawing to (read FBO is not ready).
  SwapDrawFBO,   //!< Swap the draw and read FBO (read FBO now contains previous draw FBO).
  Background,    //!< Render background without writing to the depth buffer.
  Normal,        //!< Render normal 3D geometry.
  Transparency,  //!< Render transparent 3D geometry.
  FinishDrawFBO, //!< Swap the draw into the read FBO and bind the default FBO.
  Text,          //!< Render text on top of scene.
  GUI,           //!< Render UI on top of scene and text.
  Picking,       //!< Picking render.
  Custom1,       //!< See map->setCustomRenderPass() for further details.
  Custom2,       //!< See map->setCustomRenderPass() for further details.
  Custom3,       //!< See map->setCustomRenderPass() for further details.
  Custom4        //!< See map->setCustomRenderPass() for further details.
};

//##################################################################################################
enum class ShaderType
{
  Render,
  RenderExtendedFBO,
  Picking,
  Light
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
enum class CreateColorBuffer
{
  No,
  Yes
};

//##################################################################################################
enum class Multisample
{
  No,
  Yes
};

//##################################################################################################
enum class HDR
{
  No, //!< 8 bit FBO buffers.
  Yes //!< floating point FBO buffers.
};

//##################################################################################################
enum class ExtendedFBO
{
  No, //!< FBOs will just have color and depth buffers.
  Yes //!< FBOs will also have normals and specular buffers.
};

//##################################################################################################
enum class Alpha
{
  No,
  Yes
};

//##################################################################################################
//! Replace key with value in result.
void replace(std::string& result, const std::string& key, const std::string& value);

//##################################################################################################
//! Replace light index and levels
/*!
\param lightIndex will replace %
\param levels will replace @
\param pattern containing the %'s and @'s to be replaced
\return The pattern with the %'s and @'s to be replaced.
 */
std::string replaceLight(const std::string& lightIndex, const std::string& levels, const std::string& pattern);

//##################################################################################################
//! Performs string replacement on the shader string to make it compatible with the given GLSL version.
std::string parseShaderString(const std::string& text, OpenGLProfile openGLProfile, ShaderType shaderType);

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT ShaderString
{
  TP_NONCOPYABLE(ShaderString);
  ShaderString(const char* text);
  const char* data(OpenGLProfile openGLProfile, ShaderType shaderType);

private:
  const std::string m_str;
  std::unordered_map<OpenGLProfile, std::unordered_map<ShaderType, std::string>> m_parsed;
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT ShaderResource
{
  TP_NONCOPYABLE(ShaderResource);
  ShaderResource(const std::string& resourceName);
  const char* data(OpenGLProfile openGLProfile, ShaderType shaderType);
  const std::string& dataStr(OpenGLProfile openGLProfile, ShaderType shaderType);

private:
  const std::string m_resourceName;
  std::unordered_map<OpenGLProfile, std::unordered_map<ShaderType, std::string>> m_parsed;
};

//################################################################################################
struct Matrices
{
  glm::mat4 v{1.0f};    //!< View matrix.
  glm::mat4 p{1.0f};    //!< Projection matrix.

  glm::mat4 vp{1.0f};   //!< Projection * View matrix.

  // The double precision matrices are not always set.
  glm::dmat4 dv{1.0};  //!< View matrix. (double)
  glm::dmat4 dp{1.0};  //!< Projection matrix. (double)

  glm::dmat4 dvp{1.0}; //!< Projection * View matrix. (double)

  glm::vec3 cameraOriginNear{0.0f, 0.0f, 0.0f};
  glm::vec3 cameraOriginFar {0.0f, 0.0f, 1.0f};
};


//##################################################################################################
//! Use to describe if the lighting calculation has changed or just its variables.
/*!
For efficiency the calculations for each light get compiled into the shaders as a result if we
change the numbers of lights or the type of any light we will need to recompile the shaders. If we
are just changing the parameters of a light then we do not need to recompile the shaders.
*/
enum class LightingModelChanged
{
  Yes, //!< The number of lights or the types of lights used changed.
  No   //!< Just the parameters of the lights changed.
};

//##################################################################################################
struct FBO
{
  GLuint frameBuffer{0};

  GLuint textureID{0};  //!< The color buffer texture.
  GLuint depthID{0};    //!< The depth buffer texture.

  // These are used for deferred rendering, useful for SSR and post processing. Only created if
  // using ExtendedFBO::Yes.
  GLuint normalsID{0};  //!< The normals of each fragment, useful for ray marching.
  GLuint specularID{0}; //!< The specular colors of each fragment, as well as the shininess in the alpha.

  size_t width{1};
  size_t height{1};
  size_t levels{1}; //!< Number of levels in the 3D texture generated for shadow maps.
  size_t level{0};  //!< The level that we are currently rendering, when rendering shadows.
  size_t samples{1};

#ifdef TP_ENABLE_MULTISAMPLE_FBO
  GLuint multisampleFrameBuffer{0};
  GLuint multisampleTextureID{0};

  GLuint multisampleColorRBO{0};
  GLuint multisampleDepthRBO{0};

  GLuint multisampleNormalsTextureID{0};
  GLuint multisampleSpecularTextureID{0};

  GLuint multisampleNormalsRBO{0};
  GLuint multisampleSpecularRBO{0};
#endif

  //There will be 1 for each level
  std::vector<Matrices> worldToTexture; //!< For lighting this is used to map world coords onto the texture, per level.

  Multisample multisample{Multisample::No}; //!< Yes if multisample buffers have been created.
  HDR hdr{HDR::No};                         //!< Yes if HDR buffers have been created.
  ExtendedFBO extendedFBO{ExtendedFBO::No}; //!< Yes if deferred rendering buffers have been created.
};

}

#endif
