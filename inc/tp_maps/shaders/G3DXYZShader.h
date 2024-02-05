#ifndef tp_maps_XYZShader_h
#define tp_maps_XYZShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing images.
class TP_MAPS_EXPORT G3DXYZShader: public Geometry3DShader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return xyzShaderSID();}

  //################################################################################################
  G3DXYZShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~G3DXYZShader() override;

  //################################################################################################
  void setMatrix(const glm::mat4& m, const glm::mat4& mvp);

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
