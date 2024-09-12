#ifndef tp_maps_G3DStaticLightShader_h
#define tp_maps_G3DStaticLightShader_h

#include "tp_maps/shaders/G3DMaterialShader.h"

namespace tp_math_utils
{
struct Material;
}

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing textured surfaces.
class TP_MAPS_EXPORT G3DStaticLightShader: public G3DMaterialShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return staticLightShaderSID();}

  //################################################################################################
  G3DStaticLightShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~G3DStaticLightShader() override;

  //################################################################################################
  bool initPass(RenderInfo& renderInfo,
                const Matrices& m,
                const glm::mat4& modelToWorldMatrix) override;

  //################################################################################################
  //! Call this to set the model, view, and projection matrices before drawing the geometry.
  void setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p) override;

  //################################################################################################
  void setMaterial(const tp_math_utils::OpenGLMaterial& material, const glm::mat3& uvMatrix) override;

  //################################################################################################
  /*!
  \param rgbaTextureID albedo and alpha texture.
  \param normalsTextureID normals texture.
  \param rmttrTextureID roughness, metalness, transmission, transmission roughness texture.
  */
  void setTextures(GLuint rgbaTextureID,
                   GLuint normalsTextureID,
                   GLuint rmttrTextureID) override;

  //################################################################################################
  //! Discard alpha values less than this
  /*!
  This can be used to discard alpha values on the normal render but draw them on the transparency
  pass. Typically 0.99 would be used for render and 0.01 would be used for transparency passes.
  \param discardOpacity A value between 1 and 0.
  */
  void setDiscardOpacity(float discardOpacity) override;

protected:
  //################################################################################################
  const std::string& vertexShaderStr(ShaderType shaderType) override;

  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void bindLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void init() override;
};

}

#endif
