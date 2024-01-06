#ifndef tp_maps_ImageShader_h
#define tp_maps_ImageShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing images.
class TP_MAPS_EXPORT G3DImageShader: public Geometry3DShader
{
  TP_DQ;
public:  
  //################################################################################################
  static inline const tp_utils::StringID& name(){return imageShaderSID();}

  //################################################################################################
  G3DImageShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~G3DImageShader() override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  //! Set the texture that will be drawn, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color);

  //################################################################################################
  void drawPicking(GLenum mode, VertexBuffer* vertexBuffer);

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
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  const char* vertexShaderStr(ShaderType shaderType) override;

  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void bindLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};

}

#endif
