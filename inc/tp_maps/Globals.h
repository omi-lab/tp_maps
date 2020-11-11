#ifndef tp_maps_Globals_h
#define tp_maps_Globals_h

#include "tp_utils/StringID.h"

#include "tp_math_utils/Geometry3D.h"

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

using TPGLsize = GLuint;
using TPGLfloat = float;
using TPGLenum = GLenum;
//using TPGLenum  = GLint;
#endif

#ifdef TP_GLES2 //----------------------------------------------------------------------------------
#  define TP_DEFAULT_PROFILE tp_maps::OpenGLProfile::VERSION_100_ES

#  define TP_GL_DEPTH_COMPONENT32 GL_DEPTH_COMPONENT16
#  define TP_GL_DEPTH_COMPONENT24 GL_DEPTH_COMPONENT16
#  define TP_GL_DRAW_FRAMEBUFFER GL_FRAMEBUFFER

using TPGLsize = GLsizei;
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
TP_DECLARE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DECLARE_ID(                   fontShaderSID,                      "Font shader");
TP_DECLARE_ID(                  frameShaderSID,                     "Frame shader");
TP_DECLARE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DECLARE_ID(                   gizmoLayerSID,                      "Gizmo layer");

const int32_t      UP_KEY        = 82;
const int32_t      LEFT_KEY      = 80;
const int32_t      RIGHT_KEY     = 79;
const int32_t      DOWN_KEY      = 81;

const int32_t      W_KEY         = 26;
const int32_t      A_KEY         = 4;
const int32_t      S_KEY         = 22;
const int32_t      D_KEY         = 7;

const int32_t      SPACE_KEY     = 44;

const int32_t      L_SHIFT_KEY   = 229;
const int32_t      R_SHIFT_KEY   = 225;

const int32_t      L_CTRL_KEY    = 224;

const int32_t      PAGE_UP_KEY   = 75;
const int32_t      PAGE_DOWN_KEY = 78;


//##################################################################################################
int staticInit();

//##################################################################################################
enum class RenderPass
{
  LightFBOs,     //!< Render depth maps from the point of view of lights to FBOs.
  ReflectionFBO, //!< Enable reflection FBO before rendering (Background, Normal, Transparency).
  Background,    //!< Render background without writing to the depth buffer.
  Normal,        //!< Render normal 3D geometry.
  Transparency,  //!< Render transparent 3D geometry.
  Reflection,    //!< Render reflective surfaces, map->reflectionTexture() should now be valid.
  Text,          //!< Render text on top of scene.
  GUI,           //!< Render UI on top of scene and text.
  Picking,       //!< Picking render.
  Custom1,       //!< See map->setCustomRenderPass() for further details.
  Custom2,       //!< See map->setCustomRenderPass() for further details.
  Custom3,       //!< See map->setCustomRenderPass() for further details.
  Custom4        //!< See map->setCustomRenderPass() for further details.
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
struct TP_MAPS_SHARED_EXPORT OpenGLConfig
{
  OpenGLProfile profile{TP_DEFAULT_PROFILE};
  OpenGLDepth depth{OpenGLDepth::ON_24};
};

//##################################################################################################
std::string parseShaderString(const std::string& text, OpenGLProfile openGLProfile);

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT ShaderString
{
  TP_NONCOPYABLE(ShaderString);
  ShaderString(const char* text);
  const char* data(OpenGLProfile openGLProfile);

private:
  const std::string m_str;
  std::unordered_map<OpenGLProfile, std::string> m_parsed;
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT ShaderResource
{
  TP_NONCOPYABLE(ShaderResource);
  ShaderResource(const std::string& resourceName);
  const char* data(OpenGLProfile openGLProfile);

private:
  const std::string m_resourceName;
  std::unordered_map<OpenGLProfile, std::string> m_parsed;
};

//################################################################################################
struct Matrices
{
  glm::mat4 v{1.0f};    //!< View matrix.
  glm::mat4 p{1.0f};    //!< Projection matrix.

  glm::mat4 vp{1.0f};   //!< Projection * View matrix.

  // The double precision matrices are not always set.
  glm::dmat4 dv{1.0f};  //!< View matrix. (double)
  glm::dmat4 dp{1.0f};  //!< Projection matrix. (double)

  glm::dmat4 dvp{1.0f}; //!< Projection * View matrix. (double)

  glm::vec3 cameraOriginNear{0.0f, 0.0f, 0.0f};
  glm::vec3 cameraOriginFar {0.0f, 0.0f, 1.0f};
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Material
{
  tp_utils::StringID name;
  glm::vec3 ambient{1.0f, 0.0f, 0.0f};  //!< mtl: Ka
  glm::vec3 diffuse{0.4f, 0.0f, 0.0f};  //!< mtl: Kd
  glm::vec3 specular{0.1f, 0.1f, 0.1f}; //!< mtl: Ks
  float shininess{32.0f};               //!< mtl: Ns 0 -> 128, 0=diffuse 128=sharp shiny reflections
  float alpha{1.0f};                    //!< mtl: d

  float ambientScale{1.0f};             //!< Multiplied by the ambient
  float diffuseScale{1.0f};             //!< Multiplied by the diffuse
  float specularScale{1.0f};            //!< Multiplied by the specular

  tp_utils::StringID ambientTexture;    //!< mtl: map_Ka
  tp_utils::StringID diffuseTexture;    //!< mtl: map_Kd
  tp_utils::StringID specularTexture;   //!< mtl: map_Ks
  tp_utils::StringID alphaTexture;      //!< mtl: map_d
  tp_utils::StringID bumpTexture;       //!< mtl: map_Bump

  //################################################################################################
  nlohmann::json saveState() const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
enum class LightType
{
  Directional,
  Global,
  Spot
};

//##################################################################################################
std::vector<std::string> lightTypes();

//##################################################################################################
LightType lightTypeFromString(const std::string& lightType);

//##################################################################################################
std::string lightTypeToString(LightType lightType);

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Light
{
  tp_utils::StringID name; //!< User visible for the light.

  LightType type{LightType::Directional};

  glm::mat4 viewMatrix{1.0}; //!< World to light

  glm::vec3 ambient {0.4f, 0.4f, 0.4f};
  glm::vec3 diffuse {0.6f, 0.6f, 0.6f};
  glm::vec3 specular{1.0f, 1.0f, 1.0f};

  float diffuseScale{1.0f};              //! Multiplied with the diffuse lighting calculation.

  // Used to calculate the fall off in brightness as we get further from a point light.
  float constant{1.0f};
  float linear{0.1f};
  float quadratic{0.1f};

  // Spot light textures can contain multiple shapes, this is used to select what to use. The
  // default image contains only a single shape.
  glm::vec2 spotLightUV{0.0f, 0.0f}; //!< The origin of the spot light texture.
  glm::vec2 spotLightWH{1.0f, 1.0f}; //!< The size of the spot light texture as a fraction.

  float near{0.1f};        //!< Near plane of the light frustum.
  float far{100.0f};       //!< Far plane of the light frustum.
  float fov{50.0f};        //!< Field of view for spot lights.
  float orthoRadius{10.0f}; //!< The radius of fixed directional light ortho projection.

  //################################################################################################
  void setPosition(const glm::vec3& position);

  //################################################################################################
  glm::vec3 position() const;

  //################################################################################################
  void setDirection(const glm::vec3& direction);

  //################################################################################################
  glm::vec3 direction() const;

  //################################################################################################
  nlohmann::json saveState() const;

  //################################################################################################
  void loadState(const nlohmann::json& j);

  //################################################################################################
  static nlohmann::json saveLights(const std::vector<Light>& lights);

  //################################################################################################
  static std::vector<Light> loadLights(const nlohmann::json& j);
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
  GLuint textureID{0};
  GLuint depthID{0};
  int width{1};
  int height{1};
  int levels{1}; //!< Number of levels in the 3D texture generated for shadow maps.
  int level{0};  //!< The level that we are currently rendering, when rendering shadows.

#ifdef TP_ENABLE_MULTISAMPLE_FBO
  GLuint multisampleFrameBuffer{0};
  GLuint multisampleTextureID{0};

  GLuint multisampleColorRBO{0};
  GLuint multisampleDepthRBO{0};
#endif

  glm::mat4 worldToTexture{1}; //!< For lighting this is used to map world coords onto the texture.
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Geometry
{
  std::vector<glm::vec2> geometry;
  glm::mat4 transform{1.0f};
  Material material;
};

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Geometry3D
{
  tp_math_utils::Geometry3D geometry;
  Material material;

  //################################################################################################
  tp_utils::StringID getName() const;
};

}

#endif
