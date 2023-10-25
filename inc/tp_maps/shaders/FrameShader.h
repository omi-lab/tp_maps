#ifndef tp_maps_FrameShader_h
#define tp_maps_FrameShader_h

#include "tp_maps/Shader.h"

#include "tp_utils/RefCount.h"

#include "glm/glm.hpp" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing 9 patch style images with stretchable parts.
class TP_MAPS_EXPORT FrameShader: public Shader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return frameShaderSID();}

  //################################################################################################
  FrameShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  ~FrameShader() override;

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
    TPGLsizei indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<GLushort>& indexes,
                                     const std::vector<Vertex>& verts) const;

  //################################################################################################
  void draw(GLenum mode,
            VertexBuffer* vertexBuffer,
            const glm::vec4& color);

  //################################################################################################
  void drawPicking(GLenum mode,
                   VertexBuffer* vertexBuffer,
                   const glm::vec4& pickingID);

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

  //################################################################################################
  void init() override;
};

}

#endif
