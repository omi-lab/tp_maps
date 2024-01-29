#ifndef tp_maps_G3DMaterialShader_h
#define tp_maps_G3DMaterialShader_h

#include "tp_maps/shaders/Geometry3DShader.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include "tp_math_utils/Light.h"

namespace tp_math_utils
{
struct Material;
}

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing textured surfaces.
class TP_MAPS_EXPORT G3DMaterialShader: public Geometry3DShader
{
  TP_DQ;
public:  
  //################################################################################################
  static inline const tp_utils::StringID& name(){return materialShaderSID();}

  //################################################################################################
  G3DMaterialShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~G3DMaterialShader() override;

  //################################################################################################
  //! Call this to set the lights before drawing the geometry
  void setLights(const std::vector<tp_math_utils::Light>& lights,
                 const std::vector<OpenGLFBO>& lightBuffers);

  //################################################################################################
  //! Call this to set the model, view, and projection matrices before drawing the geometry.
  virtual void setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  void setMaterial(const tp_math_utils::Material& material);

  //################################################################################################
  virtual void setMaterial(const tp_math_utils::Material& material, const glm::mat3& uvMatrix);

  //################################################################################################
  /*!
  \param rgbaTextureID albedo and alpha texture.
  \param normalsTextureID normals texture.
  \param rmttrTextureID roughness, metalness, transmission, transmission roughness texture.
  */
  virtual void setTextures(GLuint rgbaTextureID,
                           GLuint normalsTextureID,
                           GLuint rmttrTextureID);

  //################################################################################################
  void setBlankTextures();

  //################################################################################################
  //! Set number of shadow samples
  /*!
  Value 0 indicates a hard shadow edge. Larger values give softer shadow edges but make the shader
  run slower.
  */
  virtual void setShadowSamples(int shadowSamples);

  //################################################################################################
  //! Discard alpha values less than this
  /*!
  This can be used to discard alpha values on the normal render but draw them on the transparency
  pass. Typically 0.99 would be used for render and 0.01 would be used for transparency passes.
  \param discardOpacity A value between 1 and 0.
  */
  virtual void setDiscardOpacity(float discardOpacity);

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer);

  //################################################################################################
  void drawPicking(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& pickingID);

  //################################################################################################
  void initPass(RenderInfo& renderInfo,
                const Matrices& m,
                const glm::mat4& modelToWorldMatrix) override;

  //################################################################################################
  void setMaterial(RenderInfo& renderInfo,
                   const ProcessedGeometry3D& processedGeometry3D) override;

  //################################################################################################
  void setMaterialPicking(RenderInfo& renderInfo,
                          const ProcessedGeometry3D& processedGeometry3D) override;

  //################################################################################################
  void draw(RenderInfo& renderInfo,
            const ProcessedGeometry3D& processedGeometry3D,
            GLenum mode,
            VertexBuffer* vertexBuffer) override;

  //################################################################################################
  void drawPicking(RenderInfo& renderInfo,
                   const ProcessedGeometry3D& processedGeometry3D,
                   GLenum mode,
                   VertexBuffer* vertexBuffer,
                   const glm::vec4& pickingID) override;


  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  void compile(ShaderType shaderType) override;

  //################################################################################################
  const char* vertexShaderStr(ShaderType shaderType) override;

  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void bindLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void invalidate() override;

  //################################################################################################
  void drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer);
};

}

#endif
