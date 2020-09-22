#ifndef tp_maps_FrameShader_h
#define tp_maps_FrameShader_h

#include "tp_maps/Shader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing 9 patch style images with stretchable parts.
class TP_MAPS_SHARED_EXPORT FrameShader: public Shader
{
  friend class Map;
public:
  //################################################################################################
  FrameShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader=nullptr, const char* fragmentShader=nullptr);

  //################################################################################################
  ~FrameShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  //! Uses this to scale the stretchable bits of the image
  void setScale(const glm::vec3& scale);

  //################################################################################################
  //! Set the texture that will be draw, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  struct Vertex
  {
    glm::vec3 positionP; //!< A position added to positionR once it has been multiplied by the scale.
    glm::vec3 positionR; //!< A position multiplied by the scale.
    glm::vec3 normal;
    glm::vec2 texture;

    Vertex(){}
    Vertex(const glm::vec3& positionP_,
           const glm::vec3& positionR_,
           const glm::vec3& normal_,
           const glm::vec2& texture_):
      positionP(positionP_),
      positionR(positionR_),
      normal(normal_),
      texture(texture_)
    {

    }
  };

  //################################################################################################
  struct VertexBuffer
  {
    TP_REF_COUNT_OBJECTS("FrameShader::VertexBuffer");

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

    GLuint vertexCount{0};
    TPGLsize indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<GLushort>& indexes,
                                     const std::vector<Vertex>& verts) const;

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param vertices The points that make up the line.
  */
  void draw(GLenum mode,
                 VertexBuffer* vertexBuffer,
                 const glm::vec4& color);

  //################################################################################################
  //! Call this to draw the image for picking
  /*!
  \param vertices The points that make up the line.
  */
  void drawPicking(GLenum mode,
                        VertexBuffer* vertexBuffer,
                        const glm::vec4& pickingID);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return frameShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
