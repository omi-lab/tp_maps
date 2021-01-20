#include "tp_maps/Globals.h"

#include "tp_math_utils/JSONUtils.h"

#include "tp_utils/Resources.h"
#include "tp_utils/JSONUtils.h"

//##################################################################################################
namespace tp_maps
{
TP_DEFINE_ID(                      defaultSID,                          "Default");
TP_DEFINE_ID(                       screenSID,                           "Screen");
TP_DEFINE_ID(                   gizmoLayerSID,                      "Gizmo layer");
TP_DEFINE_ID(                   lineShaderSID,                      "Line shader");
TP_DEFINE_ID(                  imageShaderSID,                     "Image shader");
TP_DEFINE_ID(                image3DShaderSID,                  "Image 3D shader");
TP_DEFINE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DEFINE_ID(               materialShaderSID,                  "Material shader");
TP_DEFINE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DEFINE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DEFINE_ID(           depthImage3DShaderSID,            "Depth image 3D shader");
TP_DEFINE_ID(                   fontShaderSID,                      "Font shader");
TP_DEFINE_ID(                  frameShaderSID,                     "Frame shader");
TP_DEFINE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DEFINE_ID(                postSSRShaderSID,                  "Post ssr shader");
TP_DEFINE_ID(               postBlitShaderSID,                 "Post blit shader");

//##################################################################################################
int tp_rc();
int staticInit()
{
  // Hack to make sure that resources are loaded on OSX, this should be part of the default static
  // init as part of the build system.
  return tp_rc();
}

//##################################################################################################
void replace(std::string& result, const std::string& key, const std::string& value)
{
  size_t pos = result.find(key);
  while(pos != std::string::npos)
  {
    result.replace(pos, key.size(), value);
    pos = result.find(key, pos + value.size());
  }
}

//##################################################################################################
std::string replaceLight(const std::string& lightIndex, const std::string& levels, const std::string& pattern)
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
  replace('@', levels);
  return result;
}

//##################################################################################################
std::string parseShaderString(const std::string& text, OpenGLProfile openGLProfile, ShaderType shaderType)
{
  std::string result(text);

  auto replace = [&](const std::string& key, const std::string& value)
  {
    tp_maps::replace(result, key, value);
  };

  auto replaceRC = [&](const std::string& key, const std::string& file, ShaderType shaderType_)
  {
    if(shaderType == shaderType_)
      replace(key, tp_utils::resource("/tp_maps/"+file).data);
  };

  switch(openGLProfile)
  {
  case OpenGLProfile::VERSION_110:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 110\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 110\n#define NO_TEXTURE3D\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_120:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 120\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 120\n#define NO_TEXTURE3D\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture2D");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture3D");
    break;
  }

  case OpenGLProfile::VERSION_130:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 130\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 130\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_140:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 140\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 140\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_150:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.150.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 150\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 150\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_330:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 330\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 330\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_400:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 400\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 400\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_410:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 410\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 410\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_420:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 420\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 420\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_430:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 430\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 430\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_440:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 440\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 440\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_450:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 450\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 450\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_460:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 460\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 460\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "layout(location = 0) out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_100_ES:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.100.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 100\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 100\nprecision highp float;\n#define NO_TEXTURE3D\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture2D");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture3D");
    break;
  }

  case OpenGLProfile::VERSION_300_ES:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 300 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 300 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "layout(location = 0) out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_310_ES:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 310 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 310 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "layout(location = 0) out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }

  case OpenGLProfile::VERSION_320_ES:
  {
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.render.glsl", ShaderType::Render   );
    replaceRC("/*TP_WRITE_FRAGMENT*/", "WriteFragment.hdr.glsl"   , ShaderType::RenderHDR);
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 320 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 320 es\nprecision highp float;\nprecision highp sampler3D;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "layout(location = 0) out vec4 fragColor;");
    replace("/*TP_GLSL_TEXTURE_2D*/",        "texture");
    replace("/*TP_GLSL_TEXTURE_3D*/",        "texture");
    break;
  }
  }

  return result;
}

//##################################################################################################
ShaderString::ShaderString(const char* text):
  m_str(text)
{

}

//##################################################################################################
const char* ShaderString::data(OpenGLProfile openGLProfile, ShaderType shaderType)
{
  std::string& parsed = m_parsed[openGLProfile][shaderType];

  if(parsed.empty())
    parsed = parseShaderString(m_str, openGLProfile, shaderType);

  return parsed.c_str();
}

//##################################################################################################
ShaderResource::ShaderResource(const std::string& resourceName):
  m_resourceName(resourceName)
{

}

//##################################################################################################
const char* ShaderResource::data(OpenGLProfile openGLProfile, ShaderType shaderType)
{
  return dataStr(openGLProfile, shaderType).c_str();
}

//##################################################################################################
const std::string& ShaderResource::dataStr(OpenGLProfile openGLProfile, ShaderType shaderType)
{
  std::string& parsed = m_parsed[openGLProfile][shaderType];

  if(parsed.empty())
    parsed = parseShaderString(tp_utils::resource(m_resourceName).data, openGLProfile, shaderType);

  return parsed;
}

}
