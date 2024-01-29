#include "tp_maps/Shader.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/RefCount.h"

#include <unordered_map>

namespace tp_maps
{
namespace
{
struct ShaderDetails
{
  GLuint vertexShader{0};
  GLuint fragmentShader{0};
  GLuint program{0};
};
}

std::string Shader::vertSrcScratch;
std::string Shader::fragSrcScratch;

//##################################################################################################
struct Shader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Shader::Private");
  TP_NONCOPYABLE(Private);

  Map* map;
  tp_maps::ShaderProfile shaderProfile;
  std::unordered_map<ShaderType, ShaderDetails> shaders;
  bool error{false};
  ShaderType currentShaderType{ShaderType::Render};

  Private(Map* map_, tp_maps::ShaderProfile profile_):
    map(map_),
    shaderProfile(profile_)
  {

  }

  //################################################################################################
  void printSrc(const char* shaderSrc)
  {
    tpWarning() << "----- Shader Src -----";
    std::vector<std::string> lines;
    tpSplit(lines, shaderSrc,'\n');
    for(size_t l=0; l<lines.size(); l++)
    {
      std::string lineNumber = std::to_string(l+1);
      tp_utils::rightJustified(lineNumber, 4);
      tpWarning() << lineNumber << ": " << lines.at(l);
    }
    tpWarning() << "----------------------";
  }

};

//##################################################################################################
Shader::Shader(Map* map, tp_maps::ShaderProfile shaderProfile):
  d(new Private(map, shaderProfile))
{

}

//##################################################################################################
Shader::~Shader()
{
  for(auto i : m_pointers)
    i->m_shader = nullptr;

  for(const auto& i : d->shaders)
  {
    if(i.second.program)
      glDeleteProgram(i.second.program);

    if(i.second.vertexShader)
      glDeleteShader(i.second.vertexShader);

    if(i.second.fragmentShader)
      glDeleteShader(i.second.fragmentShader);
  }

  delete d;
}

//##################################################################################################
Map* Shader::map() const
{
  return d->map;
}

//##################################################################################################
tp_maps::ShaderProfile Shader::shaderProfile() const
{
  return d->shaderProfile;
}
//##################################################################################################
ShaderType Shader::currentShaderType() const
{
  return d->currentShaderType;
}

//##################################################################################################
GLuint Shader::loadShader(const char* shaderSrc, GLenum type)
{
  if(!shaderSrc)
  {
    tpWarning() << "Null shader string.";
    return 0;
  }

  GLuint shader = glCreateShader(type);
  if(shader == 0)
  {
    tpWarning() << "Failed to create shader.";
    return 0;
  }

  glShaderSource(shader, 1, &shaderSrc, nullptr);
  glCompileShader(shader);

  GLint compiled=0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if(!compiled)
  {
    GLchar infoLog[4096];
    glGetShaderInfoLog(shader, 4096, nullptr, static_cast<GLchar*>(infoLog));
    tpWarning() << "Failed to compile shader: " << static_cast<const GLchar*>(infoLog);

    d->printSrc(shaderSrc);

    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

//##################################################################################################
bool Shader::error() const
{
  return d->error;
}

//##################################################################################################
void Shader::use(ShaderType shaderType)
{
  if(d->shaders.find(shaderType) == d->shaders.end())
    tpWarning() << "Missing shader type " << int(shaderType);

  d->currentShaderType = shaderType;
  glUseProgram(d->shaders[shaderType].program);
}

//##################################################################################################
void Shader::compile(ShaderType shaderType)
{
  d->currentShaderType = shaderType;
  auto& s = d->shaders[shaderType];

  auto vertexShaderStr   = this->vertexShaderStr(shaderType);
  auto fragmentShaderStr = this->fragmentShaderStr(shaderType);

  s.vertexShader   = loadShader(vertexShaderStr,   GL_VERTEX_SHADER  );
  s.fragmentShader = loadShader(fragmentShaderStr, GL_FRAGMENT_SHADER);
  s.program = glCreateProgram();

  if(s.vertexShader==0 || s.fragmentShader==0 || s.program==0)
  {
    auto version = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    tpWarning() << "Error Shader::compile"
                   " d->vertexShader:" << s.vertexShader <<
                   " d->fragmentShader:" << s.fragmentShader <<
                   " d->program:" << s.program <<
                   " GL_SHADING_LANGUAGE_VERSION:" << (version?version:"");
    d->error=true;
    return;
  }

#if 0
  tp_utils::ElapsedTimer printSlowShaderCompileTimer;
  printSlowShaderCompileTimer.start();
  TP_CLEANUP([&]
  {
    if(printSlowShaderCompileTimer.elapsed()>100)
      tpWarning() << "Slow shader compile:\n" <<
                     "================== vert ==================\n" << vertexShaderStr <<
                     "\n================== frag ==================\n" << fragmentShaderStr;
  });
#endif

  glAttachShader(s.program, s.vertexShader);
  glAttachShader(s.program, s.fragmentShader);
  bindLocations(s.program, shaderType);
  glLinkProgram(s.program);

  GLint linked;
  glGetProgramiv(s.program, GL_LINK_STATUS, &linked);
  if(!linked)
  {
    GLchar infoLog[4096];
    glGetProgramInfoLog(s.program, 4096, nullptr, static_cast<GLchar*>(infoLog));
    tpWarning() << "Failed to link program: " << static_cast<const GLchar*>(infoLog);

    d->printSrc(vertexShaderStr);
    d->printSrc(fragmentShaderStr);

    glDeleteProgram(s.program);
    s.program = 0;
    d->error = true;
    return;
  }

  getLocations(s.program, shaderType);
}


//##################################################################################################
const char* Shader::vertexShaderStr(ShaderType shaderType)
{
  TP_UNUSED(shaderType);
  return "";
}

//##################################################################################################
const char* Shader::fragmentShaderStr(ShaderType shaderType)
{
  TP_UNUSED(shaderType);
  return "";
}

//##################################################################################################
void Shader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(program);
  TP_UNUSED(shaderType);
}

//##################################################################################################
void Shader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(program);
  TP_UNUSED(shaderType);
}

//##################################################################################################
void Shader::init()
{

}

//##################################################################################################
void Shader::invalidate()
{
  d->shaders.clear();
}

//##################################################################################################
void Shader::getLocation(GLuint program, GLint& location, const char* name)
{
  location = glGetUniformLocation(program, name);
#if 0
  if(location<0)
  {
    tpWarning() << "=====================================================================";
    tpWarning() << "Failed to get uniform: " << name;
    tpWarning() << "Shader type: " << shaderTypeToString(d->currentShaderType);
    tpWarning() << "Vert:";
    d->printSrc(vertexShaderStr(d->currentShaderType));
    tpWarning() << "Frag:";
    d->printSrc(fragmentShaderStr(d->currentShaderType));
    tpWarning() << "=====================================================================";
  }
#endif
};

//##################################################################################################
ShaderPointer::ShaderPointer(const Shader* shader):
  m_shader(shader)
{
  m_shader->m_pointers.push_back(this);
}

//##################################################################################################
ShaderPointer::~ShaderPointer()
{
  if(m_shader)
    tpRemoveOne(m_shader->m_pointers, this);
}

//##################################################################################################
const Shader* ShaderPointer::shader() const
{
  return m_shader;
}

}
