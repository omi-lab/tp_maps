#ifndef tp_maps_XYZShader_h
#define tp_maps_XYZShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing images.
class TP_MAPS_SHARED_EXPORT XYZShader: public Geometry3DShader
{
  friend class Map;
public:
  //################################################################################################
  XYZShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~XYZShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  void setMatrix(const glm::mat4& m, const glm::mat4& mvp);

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
  static inline const tp_utils::StringID& name(){return xyzShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
