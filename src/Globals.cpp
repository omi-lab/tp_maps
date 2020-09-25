#include "tp_maps/Globals.h"

#include "tp_math_utils/JSONUtils.h"

#include "tp_utils/Resources.h"
#include "tp_utils/JSONUtils.h"

//##################################################################################################
namespace tp_maps
{
TP_DEFINE_ID(                      defaultSID,                          "Default");
TP_DEFINE_ID(                       screenSID,                           "Screen");
TP_DEFINE_ID(                   lineShaderSID,                      "Line shader");
TP_DEFINE_ID(                  imageShaderSID,                     "Image shader");
TP_DEFINE_ID(            pointSpriteShaderSID,              "Point sprite shader");
TP_DEFINE_ID(               materialShaderSID,                  "Material shader");
TP_DEFINE_ID(               yuvImageShaderSID,                 "YUV image shader");
TP_DEFINE_ID(             depthImageShaderSID,               "Depth image shader");
TP_DEFINE_ID(                   fontShaderSID,                      "Font shader");
TP_DEFINE_ID(                  frameShaderSID,                     "Frame shader");
TP_DEFINE_ID(               postSSAOShaderSID,                 "Post ssao shader");
TP_DEFINE_ID(                   gizmoLayerSID,                      "Gizmo layer");

//##################################################################################################
int tp_rc();
int staticInit()
{
  // Hack to make sure that resources are loaded on OSX, this shoul be part of the default static
  // init as part of the build system.
  return tp_rc();
}

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
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 110\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 110\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_120:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 120\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 120\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE*/",           "texture2D");
    break;
  }

  case OpenGLProfile::VERSION_130:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 130\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 130\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_140:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 140\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 140\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_150:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 150\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 150\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_330:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 330\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 330\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_400:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 400\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 400\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_410:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 410\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 410\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_420:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 420\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 420\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_430:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 430\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 430\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_440:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 440\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 440\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_450:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 450\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 450\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_460:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 460\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 460\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_100_ES:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 100\nprecision highp float;\n");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 100\nprecision highp float;\n");
    replace("/*TP_GLSL_IN_V*/",              "attribute ");
    replace("/*TP_GLSL_IN_F*/",              "varying ");
    replace("/*TP_GLSL_OUT_V*/",             "varying ");
    replace("/*TP_GLSL_OUT_F*/",             "varying ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "gl_FragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "gl_FragColor=vec4(0,0,0,1);");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "");
    replace("/*TP_GLSL_TEXTURE*/",           "texture2D");
    break;
  }

  case OpenGLProfile::VERSION_300_ES:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 300 es\nprecision highp float;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 300 es\nprecision highp float;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_310_ES:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 310 es\nprecision highp float;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 310 es\nprecision highp float;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
    break;
  }

  case OpenGLProfile::VERSION_320_ES:
  {
    replace("/*TP_VERT_SHADER_HEADER*/",     "#version 320 es\nprecision highp float;");
    replace("/*TP_FRAG_SHADER_HEADER*/",     "#version 320 es\nprecision highp float;");
    replace("/*TP_GLSL_IN_V*/",              "in ");
    replace("/*TP_GLSL_IN_F*/",              "in ");
    replace("/*TP_GLSL_OUT_V*/",             "out ");
    replace("/*TP_GLSL_OUT_F*/",             "out ");
    replace("/*TP_GLSL_GLFRAGCOLOR*/",       "fragColor");
    replace("/*TP_GLSL_GLFRAGCOLOR_EMPTY*/", "");
    replace("/*TP_GLSL_GLFRAGCOLOR_DEF*/",   "out vec4 fragColor;\n");
    replace("/*TP_GLSL_TEXTURE*/",           "texture");
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
const char* ShaderString::data(OpenGLProfile openGLProfile)
{
  std::string& parsed = m_parsed[openGLProfile];

  if(parsed.empty())
    parsed = parseShaderString(m_str, openGLProfile);

  return parsed.c_str();
}

//##################################################################################################
ShaderResource::ShaderResource(const std::string& resourceName):
  m_resourceName(resourceName)
{

}

//##################################################################################################
const char* ShaderResource::data(OpenGLProfile openGLProfile)
{
  std::string& parsed = m_parsed[openGLProfile];

  if(parsed.empty())
    parsed = parseShaderString(tp_utils::resource(m_resourceName).data, openGLProfile);

  return parsed.c_str();
}

//##################################################################################################
std::vector<std::string> lightTypes()
{
  return {"Directional", "Global", "Spot"};
}

//##################################################################################################
LightType lightTypeFromString(const std::string& lightType)
{
  if(lightType == "Directional")
    return LightType::Directional;

  if(lightType == "Global")
    return LightType::Global;

  if(lightType == "Spot")
    return LightType::Spot;

  return LightType::Directional;
}

//##################################################################################################
std::string lightTypeToString(LightType lightType)
{
  switch(lightType)
  {
  case LightType::Directional: return "Directional";
  case LightType::Global:      return "Global";
  case LightType::Spot:        return "Spot";
  }
  return "Directional";
}

//##################################################################################################
void Light::setPosition(const glm::vec3& position)
{
  auto m = glm::inverse(viewMatrix);
  m[3] = glm::vec4(position, 1.0f);
  viewMatrix = glm::inverse(m);
}

//##################################################################################################
glm::vec3 Light::position() const
{
  return glm::inverse(viewMatrix)[3];
}

//##################################################################################################
void Light::setDirection(const glm::vec3& direction)
{
  viewMatrix = glm::lookAt(position(), position() + direction, glm::vec3(0.0f, 1.0f, 0.0f));
}

//##################################################################################################
glm::vec3 Light::direction() const
{
  return glm::mat3(glm::inverse(viewMatrix))*glm::vec3(0.0f, 0.0f, -1.0f);
}

//##################################################################################################
nlohmann::json Light::saveState() const
{
  nlohmann::json j;

  j["name"] = name.keyString();

  j["type"] = lightTypeToString(type);

  j["viewMatrix"] = tp_math_utils::mat4ToJSON(viewMatrix);

  j["ambient"] = tp_math_utils::vec3ToJSON(ambient);
  j["diffuse"] = tp_math_utils::vec3ToJSON(diffuse);
  j["specular"] = tp_math_utils::vec3ToJSON(specular);

  j["diffuseScale"] = diffuseScale;
  j["diffuseTranslate"] = diffuseTranslate;

  j["constant"] = constant;
  j["linear"] = linear;
  j["quadratic"] = quadratic;

  j["spotLightUV"] = tp_math_utils::vec2ToJSON(spotLightUV);
  j["spotLightWH"] = tp_math_utils::vec2ToJSON(spotLightWH);

  j["near"]        = near;
  j["far"]         = far;
  j["fov"]         = fov;
  j["orthoRadius"] = orthoRadius;

  return j;
}

//##################################################################################################
void Light::loadState(const nlohmann::json& j)
{
  name = TPJSONString(j, "name");

  type = lightTypeFromString(TPJSONString(j, "type"));

  viewMatrix = tp_math_utils::mat4FromJSON(TPJSON(j, "viewMatrix"));

  ambient = tp_math_utils::vec3FromJSON(TPJSON(j, "ambient"));
  diffuse = tp_math_utils::vec3FromJSON(TPJSON(j, "diffuse"));
  specular = tp_math_utils::vec3FromJSON(TPJSON(j, "specular"));

  diffuseScale = TPJSONFloat(j, "diffuseScale", diffuseScale);
  diffuseTranslate = TPJSONFloat(j, "diffuseTranslate", diffuseTranslate);

  constant = TPJSONFloat(j, "constant", constant);
  linear = TPJSONFloat(j, "linear", linear);
  quadratic = TPJSONFloat(j, "quadratic", quadratic);

  spotLightUV = tp_math_utils::getJSONVec2(j, "spotLightUV", spotLightUV);
  spotLightWH = tp_math_utils::getJSONVec2(j, "spotLightWH", spotLightWH);

  near        = TPJSONFloat(j, "near"       , near       );
  far         = TPJSONFloat(j, "far"        , far        );
  fov         = TPJSONFloat(j, "fov"        , fov        );
  orthoRadius = TPJSONFloat(j, "orthoRadius", orthoRadius);
}

//##################################################################################################
nlohmann::json Light::saveLights(const std::vector<Light>& lights)
{
  nlohmann::json j = nlohmann::json::array();
  for(const auto& light : lights)
    j.push_back(light.saveState());
  return j;
}

//##################################################################################################
std::vector<Light> Light::loadLights(const nlohmann::json& j)
{
  std::vector<Light> lights;
  if(j.is_array())
    for(const nlohmann::json& i : j)
      lights.emplace_back().loadState(i);
  return lights;
}

//##################################################################################################
tp_utils::StringID Geometry3D::getName() const
{
  return (!geometry.comments.empty())?tp_utils::StringID(geometry.comments.front()):material.name;
}

}
