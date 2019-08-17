#include "tp_maps/Globals.h"

//##################################################################################################
namespace tp_maps
{
TDP_DEFINE_ID(                      defaultSID,                          "Default")
TDP_DEFINE_ID(                       screenSID,                           "Screen")
TDP_DEFINE_ID(                   lineShaderSID,                      "Line shader")
TDP_DEFINE_ID(                  imageShaderSID,                     "Image shader")
TDP_DEFINE_ID(            pointSpriteShaderSID,              "Point sprite shader")
TDP_DEFINE_ID(               materialShaderSID,                  "Material shader")
TDP_DEFINE_ID(               yuvImageShaderSID,                 "YUV image shader")
TDP_DEFINE_ID(                   fontShaderSID,                      "Font shader")
TDP_DEFINE_ID(                  frameShaderSID,                     "Frame shader")

namespace
{
//##################################################################################################
std::string parseShaderString(const std::string& text, OpenGLProfile openGLProfile)
{
  std::string result(text);

  auto replace = [&](const std::string& key, const std::string& value)
  {
    size_t pos = result.find(key);
    while(pos != std::string::npos)
    {
      result.replace(pos, key.size(), value);
      pos = result.find(key, pos + value.size());
    }
  };

  switch(openGLProfile)
  {
  case OpenGLProfile::VERSION_110:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 110\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 110\n");
    replace("$TP_GLSL_IN_V$",            "attribute ");
    replace("$TP_GLSL_IN_F$",            "varying ");
    replace("$TP_GLSL_OUT_V$",           "varying ");
    replace("$TP_GLSL_OUT_F$",           "varying ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "gl_FragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_120:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 120\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 120\n");
    replace("$TP_GLSL_IN_V$",            "attribute ");
    replace("$TP_GLSL_IN_F$",            "varying ");
    replace("$TP_GLSL_OUT_V$",           "varying ");
    replace("$TP_GLSL_OUT_F$",           "varying ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "gl_FragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_130:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 130\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 130\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_140:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 140\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 140\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_150:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 150\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 150\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_330:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 330\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 330\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_400:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 400\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 400\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_410:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 410\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 410\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_420:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 420\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 420\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_430:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 430\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 430\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_440:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 440\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 440\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_450:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 450\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 450\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_460:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 460\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 460\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }

  case OpenGLProfile::VERSION_100_ES:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 100\nprecision highp float;\n");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 100\nprecision highp float;\n");
    replace("$TP_GLSL_IN_V$",            "attribute ");
    replace("$TP_GLSL_IN_F$",            "varying ");
    replace("$TP_GLSL_OUT_V$",           "varying ");
    replace("$TP_GLSL_OUT_F$",           "varying ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "gl_FragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "");
    replace("$TP_GLSL_TEXTURE$",         "texture2D");
    break;
  }

  case OpenGLProfile::VERSION_300_ES:
  {
    replace("$TP_VERT_SHADER_HEADER$",   "#version 300 es\nprecision highp float;");
    replace("$TP_FRAG_SHADER_HEADER$",   "#version 300 es\nprecision highp float;");
    replace("$TP_GLSL_IN_V$",            "in ");
    replace("$TP_GLSL_IN_F$",            "in ");
    replace("$TP_GLSL_OUT_V$",           "out ");
    replace("$TP_GLSL_OUT_F$",           "out ");
    replace("$TP_GLSL_GLFRAGCOLOR$",     "fragColor");
    replace("$TP_GLSL_GLFRAGCOLOR_DEF$", "out vec4 fragColor;\n");
    replace("$TP_GLSL_TEXTURE$",         "texture");
    break;
  }
  }

  return result;
}
}

//##################################################################################################
ShaderString::ShaderString(const char* text):
  m_str(text)
{

}

//##################################################################################################
const char* ShaderString::data(OpenGLProfile openGLProfile)
{
  std::string& parsed = m_parsed[openGLProfile];

  if(parsed.empty())
    parsed = parseShaderString(m_str, openGLProfile);

  return parsed.c_str();
}

}
