#ifndef tp_maps_Shader_h
#define tp_maps_Shader_h

#include "tp_maps/Globals.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include <functional>

namespace tp_maps
{
class Map;
class ShaderPointer;

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
  Shader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  virtual ~Shader();

  //################################################################################################
  Map* map() const;

  //################################################################################################
  tp_maps::ShaderProfile shaderProfile() const;

  //################################################################################################
  ShaderType currentShaderType() const;

  //################################################################################################
  GLuint loadShader(const std::string& shaderSrc, GLenum type);

  //################################################################################################
  bool error() const;

  //################################################################################################
  virtual void use(ShaderType shaderType);

protected:  
  //################################################################################################
  virtual void compile(ShaderType shaderType);

  //################################################################################################
  virtual const std::string& vertexShaderStr(ShaderType shaderType);

  //################################################################################################
  virtual const std::string& fragmentShaderStr(ShaderType shaderType);

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
