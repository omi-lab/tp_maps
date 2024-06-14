#ifndef tp_maps_G3DFlatColorShader_h
#define tp_maps_G3DFlatColorShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing geometry with a flat color.
class TP_MAPS_EXPORT G3DFlatColorShader: public Geometry3DShader
{
  TP_DQ;
public:  
  //################################################################################################
  static inline const tp_utils::StringID& name(){return flatColorShaderSID();}

  //################################################################################################
  G3DFlatColorShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~G3DFlatColorShader() override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color);

  //################################################################################################
  bool initPass(RenderInfo& renderInfo,
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
