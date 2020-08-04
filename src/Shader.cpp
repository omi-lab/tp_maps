#include "tp_maps/Shader.h"

#include "tp_utils/DebugUtils.h"

#include <unordered_map>

namespace tp_maps
{
namespace
{
struct ShaderDetails_lt
{
  GLuint vertexShader{0};
  GLuint fragmentShader{0};
  GLuint program{0};
};
}

//##################################################################################################
struct Shader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Shader::Private");
  TP_NONCOPYABLE(Private);

  Map* map;
  tp_maps::OpenGLProfile openGLProfile;
  std::unordered_map<ShaderType, ShaderDetails_lt> shaders;
  bool error{false};

  Private(Map* map_, tp_maps::OpenGLProfile profile_):
    map(map_),
    openGLProfile(profile_)
  {

  }
};

//##################################################################################################
Shader::Shader(Map* map,
               tp_maps::OpenGLProfile openGLProfile):
  d(new Private(map, openGLProfile))
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
tp_maps::OpenGLProfile Shader::openGLProfile() const
{
  return d->openGLProfile;
}

//##################################################################################################
void Shader::compile(const char* vertexShaderStr,
                     const char* fragmentShaderStr,
                     const std::function<void(GLuint)>& bindLocations,
                     const std::function<void(GLuint)>& getLocations,
                     ShaderType shaderType)
{
  auto& s = d->shaders[shaderType];

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

  glAttachShader(s.program, s.vertexShader);
  glAttachShader(s.program, s.fragmentShader);
  bindLocations(s.program);
  glLinkProgram(s.program);

  GLint linked;
  glGetProgramiv(s.program, GL_LINK_STATUS, &linked);
  if(!linked)
  {
    GLchar infoLog[4096];
    glGetProgramInfoLog(s.program, 4096, nullptr, static_cast<GLchar*>(infoLog));
    tpWarning() << "Failed to link program: " << static_cast<const GLchar*>(infoLog);

    glDeleteProgram(s.program);
    s.program = 0;
    d->error = true;
    return;
  }

  getLocations(s.program);
}

//##################################################################################################
void Shader::use(ShaderType shaderType)
{
  glUseProgram(d->shaders[shaderType].program);
}

//##################################################################################################
GLuint Shader::loadShader(const char* shaderSrc, GLenum type)
{
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

    glDeleteShader(shader);
    return 0;
  }

  return shader;
}

//##################################################################################################
bool Shader::error()const
{
  return d->error;
}

//##################################################################################################
void Shader::invalidate()
{
  d->shaders.clear();
}

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
