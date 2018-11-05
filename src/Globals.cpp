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
std::string parseShaderString(const char* text)
{
  std::string result(text);

  auto replace = [&](const std::string& key, const std::string& value)
  {
    size_t pos = result.find(key);
    while(pos != std::string::npos)
    {
      result.replace(pos, key.size(), value);
      pos = result.find(key, pos + key.size());
    }
  };

  replace("$TP_VERT_SHADER_HEADER$",   TP_VERT_SHADER_HEADER  );
  replace("$TP_FRAG_SHADER_HEADER$",   TP_FRAG_SHADER_HEADER  );
  replace("$TP_GLSL_IN_V$",            TP_GLSL_IN_V           );
  replace("$TP_GLSL_IN_F$",            TP_GLSL_IN_F           );
  replace("$TP_GLSL_OUT_V$",           TP_GLSL_OUT_V          );
  replace("$TP_GLSL_OUT_F$",           TP_GLSL_OUT_F          );
  replace("$TP_GLSL_GLFRAGCOLOR$",     TP_GLSL_GLFRAGCOLOR    );
  replace("$TP_GLSL_GLFRAGCOLOR_DEF$", TP_GLSL_GLFRAGCOLOR_DEF);
  replace("$TP_GLSL_TEXTURE$",         TP_GLSL_TEXTURE        );

  return result;
}
}

//##################################################################################################
ShaderString::ShaderString(const char* text):
  str(parseShaderString(text))
{

}

//##################################################################################################
const char* ShaderString::data() const
{
  return str.c_str();
}

}
