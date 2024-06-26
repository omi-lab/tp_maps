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
TP_DECLARE_ID(              flatColorShaderSID,                "Flat color shader");
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
TP_DECLARE_ID(                     selectedSID,                         "Selected");

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
    Stage = 2, //!< Start from the first RenderPass::Stage where the name matches pass and don't execute earlier passes.
    Reset = 3, //!< The render stage will be set to this after a render, ready for the next call to update.
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

  //##################################################################################################
  std::string typeToString() const;
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
    GUI3D,             //!< Render UI on top of scene and text, similar to GUI but has a depth buffer.
    GUI,               //!< Render UI on top of scene and text.
    Picking,           //!< Picking render.
    PickingGUI3D,      //!< Picking render for GUI3D geometry.
    Custom,            //!< A custom named render pass.
    Delegate,          //!< Delegate to the postLayer to populate render passes.

    Stage              //!< A custom named stage, used to render from partway through the pipeline.
  };

  RenderPassType type{RenderPassType::PreRender};
  tp_utils::StringID name{defaultSID()};
  size_t index{0};
  PostLayer* postLayer{nullptr};

  //################################################################################################
  RenderPass()
  {

  }

  //################################################################################################
  RenderPass(RenderPassType type_):
    type(type_)
  {

  }

  //################################################################################################
  RenderPass(RenderPassType type_, const tp_utils::StringID& name_):
    type(type_),
    name(name_)
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
    index(renderFromStage.index)
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
    return type == other.type && name == other.name && index == other.index;
  }

  //################################################################################################
  bool operator!=(const RenderPass& other) const
  {
    return type != other.type || name != other.name || index != other.index;
  }

  //################################################################################################
  bool operator>=(RenderPassType type_) const
  {
    return type >= type_;
  }

  //################################################################################################
  std::string getNameString() const
  {
    if(name.isValid())
      return name.toString();
    return std::to_string(index);
  }

  //################################################################################################
  std::string describe() const
  {
    switch(type)
    {
    case PreRender        : return "PreRender "        + getNameString();
    case LightFBOs        : return "LightFBOs "        + getNameString();
    case PrepareDrawFBO   : return "PrepareDrawFBO "   + getNameString();
    case SwapToFBO        : return "SwapToFBO "        + getNameString();
    case SwapToFBONoClear : return "SwapToFBONoClear " + getNameString();
    case BlitFromFBO      : return "BlitFromFBO "      + getNameString();
    case Background       : return "Background "       + getNameString();
    case Normal           : return "Normal "           + getNameString();
    case Transparency     : return "Transparency "     + getNameString();
    case FinishDrawFBO    : return "FinishDrawFBO "    + getNameString();
    case Text             : return "Text "             + getNameString();
    case GUI3D            : return "GUI3D "            + getNameString();
    case GUI              : return "GUI "              + getNameString();
    case Picking          : return "Picking "          + getNameString();
    case PickingGUI3D     : return "PickingGUI3D "     + getNameString();
    case Custom           : return "Custom "           + getNameString();
    case Delegate         : return "Delegate "         + getNameString();
    case Stage            : return "Stage "            + getNameString();
    }
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
enum class RenderMode
{
  Fast,
  Intermediate,
  Full
};

//##################################################################################################
std::string shaderTypeToString(ShaderType shaderType);

//##################################################################################################
enum class Subsystem
{
  OpenGL,
  OpenGLFixed,
  Vulkan,
  Direct3D
};

//##################################################################################################
/*
GLSL Version 	OpenGL Version 	  Date                 Shader Preprocessor
1.10.59[1]      2.0               30 April     2004    #version 110
1.20.8[2]       2.1               07 September 2006    #version 120
1.30.10[3]      3.0               22 November  2009    #version 130
1.40.08[4]      3.1               22 November  2009    #version 140
1.50.11[5]      3.2               04 December  2009    #version 150
3.30.6[6]       3.3               11 March     2010    #version 330
4.00.9[7]       4.0               24 July      2010    #version 400
4.10.6[8]       4.1               24 July      2010    #version 410
4.20.11[9]      4.2               12 December  2011    #version 420
4.30.8[10]      4.3               7 February   2013    #version 430
4.40.9[11]      4.4               16 June      2014    #version 440
4.50.7[12]      4.5               09 May       2017    #version 450
4.60.5[13]      4.6               14 June      2018    #version 460

HLSL
PS 1.0            - Unreleased 3dfx Rampage, DirectX 8
PS 1.1            - GeForce 3, DirectX 8
PS 1.2            - 3Dlabs Wildcat VP, DirectX 8.1
PS 1.3            - GeForce 4 Ti, DirectX 8.1
PS 1.4            - Radeon 8500-9250, Matrox Parhelia, DirectX 8.1
Shader Model 2.0  - Radeon 9500-9800/X300-X600, DirectX 9
Shader Model 2.0a - GeForce FX/PCX-optimized model, DirectX 9.0a
Shader Model 2.0b - Radeon X700-X850 shader model, DirectX 9.0b
Shader Model 3.0  - Radeon X1000 and GeForce 6, DirectX 9.0c
Shader Model 4.0  - Radeon HD 2000 and GeForce 8, DirectX 10
Shader Model 4.1  - Radeon HD 3000 and GeForce 200, DirectX 10.1
Shader Model 5.0  - Radeon HD 5000 and GeForce 400, DirectX 11
Shader Model 5.1  - GCN 1+, Fermi+, DirectX 12 (11_0+) with WDDM 2.0
Shader Model 6.0  - GCN 1+, Kepler+, DirectX 12 (11_0+) with WDDM 2.1
Shader Model 6.1  - GCN 1+, Kepler+, DirectX 12 (11_0+) with WDDM 2.3
Shader Model 6.2  - GCN 1+, Kepler+, DirectX 12 (11_0+) with WDDM 2.4
Shader Model 6.3  - GCN 1+, Kepler+, DirectX 12 (11_0+) with WDDM 2.5
Shader Model 6.4  - GCN 1+, Kepler+, Skylake+, DirectX 12 (11_0+) with WDDM 2.6
Shader Model 6.5  - GCN 1+, Kepler+, Skylake+, DirectX 12 (11_0+) with WDDM 2.7
Shader Model 6.6  - GCN 4+, Maxwell+, DirectX 12 (11_0+) with WDDM 3.0
Shader Model 6.7  - GCN 4+, Maxwell+, DirectX 12 (12_0+) with WDDM 3.1
*/
enum class ShaderProfile
{
  GLSL_110 = 20,  // Not really supported by most of the shaders in tp_maps
  GLSL_120 = 21,
  GLSL_130 = 30,
  GLSL_140 = 31,
  GLSL_150 = 32,
  GLSL_330 = 33,
  GLSL_400 = 40,
  GLSL_410 = 41,
  GLSL_420 = 42,
  GLSL_430 = 43,
  GLSL_440 = 44,
  GLSL_450 = 45,
  GLSL_460 = 46,
  GLSL_100_ES,
  GLSL_300_ES,
  GLSL_310_ES,
  GLSL_320_ES,
  HLSL_10,
  HLSL_11,
  HLSL_12,
  HLSL_13,
  HLSL_14,
  HLSL_20,
  HLSL_20a,
  HLSL_20b,
  HLSL_30,
  HLSL_40,
  HLSL_41,
  HLSL_50,
  HLSL_51,
  HLSL_60,
  HLSL_61,
  HLSL_62,
  HLSL_63,
  HLSL_64,
  HLSL_65,
  HLSL_66,
  HLSL_67
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
//! Replace light index
/*!
\param lightIndex will replace %
\param pattern containing the %'s and @'s to be replaced
\return The pattern with the %'s and @'s to be replaced.
 */
std::string replaceLight(const std::string& lightIndex, const std::string& pattern);

//##################################################################################################
//! Performs string replacement on the shader string to make it compatible with the given GLSL version.
std::string parseShaderString(const std::string& text, ShaderProfile shaderProfile, ShaderType shaderType);

//##################################################################################################
struct TP_MAPS_EXPORT ShaderString
{
  TP_NONCOPYABLE(ShaderString);
  ShaderString(const char* text);
  const char* data(ShaderProfile shaderProfile, ShaderType shaderType);

private:
  std::string m_str;
  std::unordered_map<ShaderProfile, std::unordered_map<ShaderType, std::string>> m_parsed;
};

//##################################################################################################
struct TP_MAPS_EXPORT ShaderResource
{
  TP_NONCOPYABLE(ShaderResource);
  ShaderResource(const std::string& resourceName);
  const char* data(ShaderProfile shaderProfile, ShaderType shaderType);
  const std::string& dataStr(ShaderProfile shaderProfile, ShaderType shaderType);

private:
  const std::string m_resourceName;
  std::unordered_map<ShaderProfile, std::unordered_map<ShaderType, std::string>> m_parsed;
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
enum class NChannels
{
  RGB  = 3,
  RGBA = 4
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
