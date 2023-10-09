#ifndef tp_maps_StaticLightShader_h
#define tp_maps_StaticLightShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

#include "tp_math_utils/Light.h"

namespace tp_math_utils
{
struct Material;
}

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing textured surfaces.
class TP_MAPS_EXPORT StaticLightShader: public Geometry3DShader
{
public:
  //################################################################################################
  StaticLightShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~StaticLightShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the model, view, and projection matrices before drawing the geometry.
  void setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  void setMaterial(const tp_math_utils::Material& material);

  //################################################################################################
  void setMaterial(const tp_math_utils::Material& material, const glm::mat3& uvMatrix);

  //################################################################################################
  /*!
  \param rgbaTextureID albedo and alpha texture.
  \param normalsTextureID normals texture.
  \param rmttrTextureID roughness, metalness, transmission, transmission roughness texture.
  */
  void setTextures(GLuint rgbaTextureID,
                   GLuint normalsTextureID,
                   GLuint rmttrTextureID);

  //################################################################################################
  void setBlankTextures();

  //################################################################################################
  //! Discard alpha values less than this
  /*!
  This can be used to discard alpha values on the normal render but draw them on the transparency
  pass. Typically 0.99 would be used for render and 0.01 would be used for transparency passes.
  \param discardOpacity A value between 1 and 0.
  */
  void setDiscardOpacity(float discardOpacity);

  //################################################################################################
  void init(RenderInfo& renderInfo,
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
  static inline const tp_utils::StringID& name(){return staticLightShaderSID();}

protected:  
  //################################################################################################
  void invalidate() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
