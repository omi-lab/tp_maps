#ifndef tp_maps_Globals_h
#define tp_maps_Globals_h

#include "tp_utils/StringID.h"

#include "lib_platform/Globals.h"

#include "glm/glm.hpp" // IWYU pragma: keep

#include <unordered_map>

#if defined(TP_MAPS_LIBRARY)
#  define TP_MAPS_EXPORT TP_EXPORT
#else
#  define TP_MAPS_EXPORT TP_IMPORT
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

#define TP_ENABLE_3D_TEXTURE

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
#  include <GL/glew.h>
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

#define      TP_A_KEY           4
#define      TP_B_KEY           5
#define      TP_C_KEY           6
#define      TP_D_KEY           7
#define      TP_E_KEY           8
#define      TP_F_KEY           9
#define      TP_G_KEY          10
#define      TP_H_KEY          11
#define      TP_I_KEY          12
#define      TP_J_KEY          13
#define      TP_K_KEY          14
#define      TP_L_KEY          15
#define      TP_M_KEY          16
#define      TP_N_KEY          17
#define      TP_O_KEY          18
#define      TP_P_KEY          19
#define      TP_Q_KEY          20
#define      TP_R_KEY          21
#define      TP_S_KEY          22
#define      TP_T_KEY          23
#define      TP_U_KEY          24
#define      TP_V_KEY          25
#define      TP_W_KEY          26
#define      TP_X_KEY          27
#define      TP_Y_KEY          28
#define      TP_Z_KEY          29

#define      TP_COMMA_KEY      -17
#define      TP_ESCAPE_KEY    16777155

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
TP_DECLARE_ID(                    xyzShaderSID,                       "XYZ shader");
TP_DECLARE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DECLARE_ID(               materialShaderSID,                  "Material shader");
TP_DECLARE_ID(            staticLightShaderSID,              "Static light shader");
TP_DECLARE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DECLARE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DECLARE_ID(           depthImage3DShaderSID,            "Depth image 3D shader");
TP_DECLARE_ID(                  depthShaderSID,                     "Depth shader");
TP_DECLARE_ID(                   fontShaderSID,                      "Font shader");
TP_DECLARE_ID(                  frameShaderSID,                     "Frame shader");
TP_DECLARE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DECLARE_ID(       ambientOcclusionShaderSID,         "Ambient occlusion shader");
TP_DECLARE_ID(  mergeAmbientOcclusionShaderSID,   "Merge ambient occlusion shader");
TP_DECLARE_ID(                postSSRShaderSID,                  "Post ssr shader");
TP_DECLARE_ID(               postBlitShaderSID,                 "Post blit shader");
TP_DECLARE_ID(            postOutlineShaderSID,              "Post outline shader");
TP_DECLARE_ID(          postBasicBlurShaderSID,           "Post basic blur shader");
TP_DECLARE_ID(        postBlurAndTintShaderSID,        "Post blur and tint shader");
TP_DECLARE_ID(       depthOfFieldBlurShaderSID,       "Depth of field blur shader");
TP_DECLARE_ID(         calculateFocusShaderSID,           "Calculate focus shader");
TP_DECLARE_ID(             downsampleShaderSID,                "Downsample shader");
TP_DECLARE_ID(               mergeDofShaderSID,                 "Merge dof shader");
TP_DECLARE_ID(            passThroughShaderSID,              "Pass through shader");
TP_DECLARE_ID(             postGrid2DShaderSID,              "Post grid 2D shader");
TP_DECLARE_ID(              postGammaShaderSID,                "Post gamma shader");
TP_DECLARE_ID(             backgroundShaderSID,                "Background shader");
TP_DECLARE_ID(        backgroundImageShaderSID,          "Background image shader");
TP_DECLARE_ID(      backgroundPatternShaderSID,        "Background pattern shader");
TP_DECLARE_ID(                selectionPassSID,                   "Selection pass");

//##################################################################################################
int staticInit();

class PostLayer;

//##################################################################################################
struct RenderFromStage
{
  //################################################################################################
  enum RenderFromStageType : size_t
  {
    Full = 1, //!< Start from the first RenderPass pass.
    RenderMoreLights = 2, //!< Start from the first RenderPass::LightFBOs pass and don't execute earlier passes.
    Stage = 3, //!< Start from the first RenderPass::Stage where the name matches pass and don't execute earlier passes.
    Reset = 4, //!< The render stage will be set to this after a render, ready for the next call to update.
  };

  RenderFromStageType type;
  size_t index{0};

  //################################################################################################
  RenderFromStage()
  {

  }

  //################################################################################################
  RenderFromStage(RenderFromStageType type_ = RenderFromStage::Full, size_t index_=0):
    type(type_),
    index(index_)
  {

  }

  //################################################################################################
  bool operator==(RenderFromStageType type_)
  {
    return type == type_;
  }

  //################################################################################################
  bool operator!=(RenderFromStageType type_)
  {
    return type != type_;
  }

  //################################################################################################
  bool operator<(RenderFromStage other)
  {
    if(type<other.type)
      return true;
    return index<other.index;
  }
};

//##################################################################################################
struct RenderPass
{
  //################################################################################################
  enum RenderPassType : size_t
  {
    PreRender,         //!< Executed at the start of a render to update models.
    LightFBOs,         //!< Render depth maps from the point of view of lights to FBOs.
    PrepareDrawFBO,    //!< Prepare the initial draw FBO ready for drawing to (read FBO is not ready).

    // Only fbo 0 is multisampled.
    SwapToFBO,         //!< Swap the draw and read FBO (draw=FBO n, read=previous draw FBO).
    SwapToFBONoClear,  //!< Same as above but does not clear the draw depth or color buffers.
    BlitFromFBO,       //!< Blit from FBO n into the current draw buffer. Multisample.

    Background,        //!< Render background without writing to the depth buffer.
    Normal,            //!< Render normal 3D geometry.
    Transparency,      //!< Render transparent 3D geometry.
    FinishDrawFBO,     //!< Swap the draw into the read FBO and bind the default FBO.
    Text,              //!< Render text on top of scene.
    GUI,               //!< Render UI on top of scene and text.
    Picking,           //!< Picking render.
    Custom,            //!< A custom named render pass.
    Delegate,          //!< Delegate to the postLayer to populate render passes.

    Stage              //!< A custom named stage, used to render from partway through the pipeline.
  };

  RenderPassType type{RenderPassType::PreRender};
  tp_utils::WeakStringID name{nullptr};
  PostLayer* postLayer{nullptr};

  //################################################################################################
  RenderPass()
  {

  }

  //################################################################################################
  RenderPass(RenderPassType type_, tp_utils::WeakStringID name_=nullptr):
    type(type_),
    name(name_)
  {

  }

  //################################################################################################
  RenderPass(RenderPassType type_, tp_utils::StringID name_):
    type(type_),
    name(name_.weak())
  {

  }

  //################################################################################################
  RenderPass(PostLayer* postLayer_):
    type(Delegate),
    postLayer(postLayer_)
  {

  }

  //################################################################################################
  RenderPass(const RenderFromStage& renderFromStage):
    type(Stage),
    name(reinterpret_cast<tp_utils::WeakStringID>(renderFromStage.index))
  {

  }

  //################################################################################################
  bool operator==(RenderPassType type_) const
  {
    return type == type_;
  }

  //################################################################################################
  bool operator!=(RenderPassType type_) const
  {
    return type != type_;
  }

  //################################################################################################
  bool operator==(const RenderPass& other) const
  {
    return type == other.type && name == other.name;
  }

  //################################################################################################
  bool operator!=(const RenderPass& other) const
  {
    return type != other.type || name != other.name;
  }

  //################################################################################################
  bool operator>=(RenderPassType type_) const
  {
    return type >= type_;
  }

  //################################################################################################
  std::string getNameString() const
  {
    if(name)
      return tp_utils::StringID::fromWeak(name).toString();
    return std::to_string(size_t(type));
  }
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
std::string shaderTypeToString(ShaderType shaderType);

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
struct TP_MAPS_EXPORT ShaderString
{
  TP_NONCOPYABLE(ShaderString);
  ShaderString(const char* text);
  const char* data(OpenGLProfile openGLProfile, ShaderType shaderType);

private:
  std::string m_str;
  std::unordered_map<OpenGLProfile, std::unordered_map<ShaderType, std::string>> m_parsed;
};

//##################################################################################################
struct TP_MAPS_EXPORT ShaderResource
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

  glm::vec2 nearAndFar() const;
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

//##################################################################################################
enum class KeyboardModifier : size_t
{
  None    = 0b00000000,
  Shift   = 0b00000001,
  Control = 0b00000010,
  Alt     = 0b00000100
};

//##################################################################################################
enum class MouseEventType
{
  Press,
  Move,
  Release,
  Wheel,
  DoubleClick,

  // Events composed from other events.
  Click,    //! The mouse has been pressed and released.
  DragStart //! The mouse has been pressed and moved.
};

//##################################################################################################
enum class Button : size_t
{
  NoButton     = 0,
  RightButton  = 1,
  LeftButton   = 2,
  MiddleButton = 4
};


//##################################################################################################
[[nodiscard]] KeyboardModifier operator|(KeyboardModifier lhs,KeyboardModifier rhs);

//##################################################################################################
[[nodiscard]] KeyboardModifier operator&(KeyboardModifier lhs,KeyboardModifier rhs);

//##################################################################################################
[[nodiscard]] bool keyboardModifierAnySet(KeyboardModifier mask, KeyboardModifier bit);

//##################################################################################################
[[nodiscard]] bool keyboardModifierAllSet(KeyboardModifier mask, KeyboardModifier bit);

//##################################################################################################
template<typename V>
inline V gammaCorrectedToLinear(const V& in)
{
  glm::vec3 out;
  out.x = std::pow(in.x, 2.2f);
  out.y = std::pow(in.y, 2.2f);
  out.z = std::pow(in.z, 2.2f);
  return out;
}

//##################################################################################################
template<typename V>
inline V linearToGammaCorrected(const V& in)
{
  glm::vec3 out;
  out.x = std::pow(in.x, 1.0f/2.2f);
  out.y = std::pow(in.y, 1.0f/2.2f);
  out.z = std::pow(in.z, 1.0f/2.2f);
  return out;
}

}

#endif
