#include "tp_maps/Globals.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/Resources.h"
#include "tp_utils/DebugUtils.h"

//##################################################################################################
namespace tp_maps
{
TP_DEFINE_ID(                      defaultSID,                          "Default");
TP_DEFINE_ID(                       screenSID,                           "Screen");
TP_DEFINE_ID(                   gizmoLayerSID,                      "Gizmo layer");
TP_DEFINE_ID(                   lineShaderSID,                      "Line shader");
TP_DEFINE_ID(                  imageShaderSID,                     "Image shader");
TP_DEFINE_ID(              flatColorShaderSID,                "Flat color shader");
TP_DEFINE_ID(                image3DShaderSID,                  "Image 3D shader");
TP_DEFINE_ID(                    xyzShaderSID,                       "XYZ shader");
TP_DEFINE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DEFINE_ID(               materialShaderSID,                  "Material shader");
TP_DEFINE_ID(            staticLightShaderSID,              "Static light shader");
TP_DEFINE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DEFINE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DEFINE_ID(                pickingShaderSID,                   "Picking shader");
TP_DEFINE_ID(           depthImage3DShaderSID,            "Depth image 3D shader");
TP_DEFINE_ID(                  depthShaderSID,                     "Depth shader");
TP_DEFINE_ID(                   fontShaderSID,                      "Font shader");
TP_DEFINE_ID(                  frameShaderSID,                     "Frame shader");
TP_DEFINE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DEFINE_ID(       ambientOcclusionShaderSID,         "Ambient occlusion shader");
TP_DEFINE_ID(  mergeAmbientOcclusionShaderSID,   "Merge ambient occlusion shader");
TP_DEFINE_ID(                postSSRShaderSID,                  "Post ssr shader");
TP_DEFINE_ID(               postBlitShaderSID,                 "Post blit shader");
TP_DEFINE_ID(            postOutlineShaderSID,              "Post outline shader");
TP_DEFINE_ID(          postBasicBlurShaderSID,           "Post basic blur shader");
TP_DEFINE_ID(        postBlurAndTintShaderSID,        "Post blur and tint shader");
TP_DEFINE_ID(       depthOfFieldBlurShaderSID,       "Depth of field blur shader");
TP_DEFINE_ID(         calculateFocusShaderSID,           "Calculate focus shader");
TP_DEFINE_ID(             downsampleShaderSID,                "Downsample shader");
TP_DEFINE_ID(               mergeDofShaderSID,                 "Merge dof shader");
TP_DEFINE_ID(           gaussianBlurShaderSID,             "Gaussian blur shader");
TP_DEFINE_ID(            passThroughShaderSID,              "Pass through shader");
TP_DEFINE_ID(             postGrid2DShaderSID,              "Post grid 2D shader");
TP_DEFINE_ID(              postGammaShaderSID,                "Post gamma shader");
TP_DEFINE_ID(             backgroundShaderSID,                "Background shader");
TP_DEFINE_ID(        backgroundImageShaderSID,          "Background image shader");
TP_DEFINE_ID(      backgroundPatternShaderSID,        "Background pattern shader");
TP_DEFINE_ID(                selectionPassSID,                   "Selection pass");
TP_DEFINE_ID(                     selectedSID,                         "Selected");

//##################################################################################################
int tp_rc();
int staticInit()
{
  // Hack to make sure that resources are loaded on OSX, this should be part of the default static
  // init as part of the build system.
  return tp_rc();
}


//##################################################################################################
std::string RenderFromStage::typeToString() const
{
  switch(type)
  {
    case RenderFromStageType::Full : return "Full";
    case RenderFromStageType::Stage : return "Stage";
    case RenderFromStageType::Reset : return "Reset";
  }
  return {};
}

//##################################################################################################
std::string shaderTypeToString(ShaderType shaderType)
{
  switch(shaderType)
  {
    case ShaderType::Render           : return "Render";
    case ShaderType::RenderExtendedFBO: return "RenderExtendedFBO";
    case ShaderType::Picking          : return "Picking";
    case ShaderType::Light            : return "Light";
  }
  return {};
}

//##################################################################################################
std::string replaceLight(const std::string& lightIndex, const std::string& pattern)
{
  std::string result = pattern;

  auto replace = [&result](char key, const std::string& value)
  {
    size_t pos = result.find(key);
    while(pos != std::string::npos)
    {
      result.replace(pos, 1, value);
      pos = result.find(key, pos + value.size());
    }
  };

  replace('%', lightIndex);
  return result;
}

//##################################################################################################
std::string parseShaderString(const std::string& text, ShaderProfile shaderProfile, ShaderType shaderType)
{
  std::string result(text);

  auto replace = [&](const std::string& key, const std::string& value)
  {
    tp_utils::replace(result, "#pragma replace " + key, value);
  };

  auto replaceRC = [&](const std::string& key, const std::string& file, ShaderType shaderType_)
  {
    if(shaderType == shaderType_)
      replace(key, tp_utils::resource("/tp_maps/"+file).data);
  };

  auto define = [&](const std::string& key, const std::string& value)
  {
    tp_utils::replace(result, "#define " + key, "#define " + key + " " + value);
  };

  switch(shaderProfile)
  {
    case ShaderProfile::GLSL_110:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 110\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 110\n");
      define ("TP_GLSL_IN_V",              "attribute ");
      define ("TP_GLSL_IN_F",              "varying ");
      define ("TP_GLSL_OUT_V",             "varying ");
      define ("TP_GLSL_GLFRAGCOLOR",       "gl_FragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_120:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 120\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 120\n");
      define ("TP_GLSL_IN_V",              "attribute ");
      define ("TP_GLSL_IN_F",              "varying ");
      define ("TP_GLSL_OUT_V",             "varying ");
      define ("TP_GLSL_GLFRAGCOLOR",       "gl_FragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "");
      define ("TP_GLSL_TEXTURE_2D",        "texture2D");
      define ("TP_GLSL_TEXTURE_3D",        "texture3D");
      break;
    }

    case ShaderProfile::GLSL_130:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 130\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 130\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_140:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 140\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 140\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_150:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 150\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 150\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_330:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 330\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 330\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_400:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 400\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 400\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_410:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 410\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 410\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_420:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 420\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 420\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_430:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 430\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 430\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_440:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 440\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 440\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_450:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 450\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 450\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_460:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 460\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 460\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "layout(location = 0) out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_100_ES:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 100\nprecision highp float;\n");
      replace("TP_FRAG_SHADER_HEADER",     "#version 100\nprecision highp float;\n");
      define ("TP_GLSL_IN_V",              "attribute ");
      define ("TP_GLSL_IN_F",              "varying ");
      define ("TP_GLSL_OUT_V",             "varying ");
      define ("TP_GLSL_GLFRAGCOLOR",       "gl_FragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "");
      define ("TP_GLSL_TEXTURE_2D",        "texture2D");
      define ("TP_GLSL_TEXTURE_3D",        "texture3D");
      break;
    }

    case ShaderProfile::GLSL_300_ES:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 300 es\nprecision highp float;\nprecision highp sampler3D;");
      replace("TP_FRAG_SHADER_HEADER",     "#version 300 es\nprecision highp float;\nprecision highp sampler3D;");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "layout(location = 0) out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_310_ES:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 310 es\nprecision highp float;\nprecision highp sampler3D;");
      replace("TP_FRAG_SHADER_HEADER",     "#version 310 es\nprecision highp float;\nprecision highp sampler3D;");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "layout(location = 0) out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::GLSL_320_ES:
    {
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.render.glsl", ShaderType::Render   );
      replaceRC("TP_WRITE_FRAGMENT", "WriteFragment.hdr.glsl"   , ShaderType::RenderExtendedFBO);
      replace("TP_VERT_SHADER_HEADER",     "#version 320 es\nprecision highp float;\nprecision highp sampler3D;");
      replace("TP_FRAG_SHADER_HEADER",     "#version 320 es\nprecision highp float;\nprecision highp sampler3D;");
      define ("TP_GLSL_IN_V",              "in ");
      define ("TP_GLSL_IN_F",              "in ");
      define ("TP_GLSL_OUT_V",             "out ");
      define ("TP_GLSL_GLFRAGCOLOR",       "fragColor");
      replace("TP_GLSL_GLFRAGCOLOR_DEF",   "layout(location = 0) out vec4 fragColor;");
      define ("TP_GLSL_TEXTURE_2D",        "texture");
      define ("TP_GLSL_TEXTURE_3D",        "texture");
      break;
    }

    case ShaderProfile::HLSL_10:
    case ShaderProfile::HLSL_11:
    case ShaderProfile::HLSL_12:
    case ShaderProfile::HLSL_13:
    case ShaderProfile::HLSL_14:
    case ShaderProfile::HLSL_20:
    case ShaderProfile::HLSL_20a:
    case ShaderProfile::HLSL_20b:
    case ShaderProfile::HLSL_30:
    case ShaderProfile::HLSL_40:
    case ShaderProfile::HLSL_41:
    case ShaderProfile::HLSL_50:
    case ShaderProfile::HLSL_51:
    case ShaderProfile::HLSL_60:
    case ShaderProfile::HLSL_61:
    case ShaderProfile::HLSL_62:
    case ShaderProfile::HLSL_63:
    case ShaderProfile::HLSL_64:
    case ShaderProfile::HLSL_65:
    case ShaderProfile::HLSL_66:
    case ShaderProfile::HLSL_67:
    {
      tpWarning() << "HLSL not implemented.";
      break;
    }
  }

  return result;
}

//##################################################################################################
ShaderString::ShaderString(const char* text)
{
  if(!text)
  {
    tpWarning() << "ShaderString initialized from null.";
    return;
  }

  m_str = text;
}

//##################################################################################################
const char* ShaderString::data(ShaderProfile shaderProfile, ShaderType shaderType)
{
  std::string& parsed = m_parsed[shaderProfile][shaderType];

  if(parsed.empty())
    parsed = parseShaderString(m_str, shaderProfile, shaderType);

  return parsed.c_str();
}

//##################################################################################################
ShaderResource::ShaderResource(const std::string& resourceName):
  m_resourceName(resourceName)
{

}

//##################################################################################################
const char* ShaderResource::data(ShaderProfile shaderProfile, ShaderType shaderType)
{
  return dataStr(shaderProfile, shaderType).c_str();
}

//##################################################################################################
const std::string& ShaderResource::dataStr(ShaderProfile shaderProfile, ShaderType shaderType)
{
  std::string& parsed = m_parsed[shaderProfile][shaderType];

  if(parsed.empty())
  {
    if(auto data = tp_utils::resource(m_resourceName).data; data)
      parsed = parseShaderString(data, shaderProfile, shaderType);
    else
      tpWarning() << "ShaderResource is null, resourceName=" << m_resourceName;
  }

  return parsed;
}

//##################################################################################################
glm::vec2 Matrices::nearAndFar() const
{
  glm::mat4 m = glm::inverse(p);
  return {std::fabs(tpProj(m, {0.0f, 0.0f, -1.0f}).z), std::fabs(tpProj(m, {0.0f, 0.0f, 1.0f}).z)};
}

//##################################################################################################
KeyboardModifier operator|(KeyboardModifier lhs,KeyboardModifier rhs)
{
  return KeyboardModifier(size_t(lhs) | size_t(rhs));
}

//##################################################################################################
KeyboardModifier operator&(KeyboardModifier lhs,KeyboardModifier rhs)
{
  return KeyboardModifier(size_t(lhs) & size_t(rhs));
}

//##################################################################################################
bool keyboardModifierAnySet(KeyboardModifier mask, KeyboardModifier bit)
{
  return (mask & bit) != KeyboardModifier::None;
}

//##################################################################################################
bool keyboardModifierAllSet(KeyboardModifier mask, KeyboardModifier bit)
{
  return (mask & bit) == bit;
}

}
