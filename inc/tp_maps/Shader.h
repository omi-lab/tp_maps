#ifndef tp_maps_Shader_h
#define tp_maps_Shader_h

#include "tp_maps/Globals.h"

#include <functional>

namespace tp_maps
{
class Map;
class ShaderPointer;

struct ShaderDetails
{
  GLuint vertexShader{0};
  GLuint fragmentShader{0};
  GLuint program{0};
};

//##################################################################################################
//! The base class for shaders.
/*!
This allows the map to cache shaders.
*/
class TP_MAPS_EXPORT Shader
{
  TP_DQ;
  TP_NONCOPYABLE(Shader);
  friend class Map;
  friend class ShaderPointer;
  mutable std::vector<ShaderPointer*> m_pointers;
public:
  //################################################################################################
  Shader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  virtual ~Shader();

  //################################################################################################
  Map* map() const;

  //################################################################################################
  tp_maps::OpenGLProfile openGLProfile() const;

  //################################################################################################
  ShaderType currentShaderType() const;

  //################################################################################################
  GLuint loadShader(const char* shaderSrc, GLenum type);

  //################################################################################################
  bool error() const;

  //################################################################################################
  ShaderDetails shaderDetails(ShaderType shaderType) const;

  //################################################################################################
  virtual void use(ShaderType shaderType);

protected:  
  //################################################################################################
  virtual void compile(ShaderType shaderType);

  //################################################################################################
  virtual const char* vertexShaderStr(ShaderType shaderType);

  //################################################################################################
  virtual const char* fragmentShaderStr(ShaderType shaderType);

  //################################################################################################
  virtual void bindLocations(GLuint program, ShaderType shaderType);

  //################################################################################################
  virtual void getLocations(GLuint program, ShaderType shaderType);

  //################################################################################################
  virtual void init();

  //################################################################################################
  virtual void invalidate();

  //################################################################################################
  void getLocation(GLuint program, GLint& location, const char* name);

  // Scratch space for composing shader program strings.
  static std::string vertSrcScratch;
  static std::string fragSrcScratch;
};

//##################################################################################################
class ShaderPointer
{
  TP_NONCOPYABLE(ShaderPointer);
  friend class Shader;
  const Shader* m_shader;
public:
  //################################################################################################
  ShaderPointer(const Shader* shader);

  //################################################################################################
  ~ShaderPointer();

  //################################################################################################
  const Shader* shader() const;
};

}

#endif
