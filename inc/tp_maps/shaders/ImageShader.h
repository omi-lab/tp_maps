#ifndef tp_maps_ImageShader_h
#define tp_maps_ImageShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing images.
class TP_MAPS_EXPORT ImageShader: public Geometry3DShader
{
  friend class Map;
public:
  //################################################################################################
  ImageShader(Map* map,
              tp_maps::OpenGLProfile openGLProfile,
              const char* vertexShader=nullptr,
              const char* fragmentShader=nullptr);

  //################################################################################################
  ~ImageShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  //! Set the texture that will be drawn, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  void setTexture3D(GLuint textureID, size_t level);

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color);

  //################################################################################################
  void drawPicking(GLenum mode, VertexBuffer* vertexBuffer);

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
  static inline const tp_utils::StringID& name(){return imageShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

//##################################################################################################
//! Used to textures with multiple levels (3D textures).
class TP_MAPS_EXPORT Image3DShader: public ImageShader
{
public:
  //################################################################################################
  Image3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return image3DShaderSID();}
};

}

#endif
