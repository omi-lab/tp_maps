#ifndef tp_maps_Geometry3DShader_h
#define tp_maps_Geometry3DShader_h

#include "tp_maps/Shader.h"

#include "tp_utils/RefCount.h"

#include "glm/glm.hpp" // IWYU pragma: keep

namespace tp_maps
{
struct ProcessedGeometry3D;
class RenderInfo;

//##################################################################################################
//! The base class for shaders drawing normal 3D geometry.
class TP_MAPS_EXPORT Geometry3DShader: public Shader
{
public:
  //################################################################################################
  using Shader::Shader;

  //################################################################################################
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texture;

    Vertex()=default;
    Vertex(const glm::vec3& position_, // vertex position
           const glm::vec3& normal_,   // vertex normal
           const glm::vec3& tangent_,  // vertex tangent
           const glm::vec2& texture_): // vertex texture coordinates
      position(position_),
      normal(normal_),
      tangent(tangent_),
      texture(texture_)
    {

    }
  };

  //################################################################################################
  struct VertexBuffer
  {
    TP_REF_COUNT_OBJECTS("Geometry3DShader::VertexBuffer");

    //##############################################################################################
    VertexBuffer(Map* map_, const Shader* shader_);

    //##############################################################################################
    ~VertexBuffer();

    //##############################################################################################
    void bindVBO() const;

    Map* map;
    ShaderPointer shader;

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    //The Vertex Array Object
    GLuint vaoID{0};

    //The Index Buffer Object
    GLuint iboID{0};
#endif

    //The Vertex Buffer Object
    GLuint vboID{0};

    GLuint   vertexCount{0};
    TPGLsizei  indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<GLuint>& indexes,
                                     const std::vector<Vertex>& verts) const;

  //################################################################################################
  virtual void initPass(RenderInfo& renderInfo,
                        const Matrices& m,
                        const glm::mat4& modelToWorldMatrix)=0;

  //################################################################################################
  virtual void setMaterial(RenderInfo& renderInfo,
                           const ProcessedGeometry3D& processedGeometry3D)=0;

  //################################################################################################
  virtual void setMaterialPicking(RenderInfo& renderInfo,
                                  const ProcessedGeometry3D& processedGeometry3D)=0;

  //################################################################################################
  virtual void draw(RenderInfo& renderInfo,
                    const ProcessedGeometry3D& processedGeometry3D,
                    GLenum mode,
                    VertexBuffer* vertexBuffer)=0;

  //################################################################################################
  virtual void drawPicking(RenderInfo& renderInfo,
                           const ProcessedGeometry3D& processedGeometry3D,
                           GLenum mode,
                           VertexBuffer* vertexBuffer,
                           const glm::vec4& pickingID)=0;

protected:
  //################################################################################################
  void init() override;
};

}

#endif
