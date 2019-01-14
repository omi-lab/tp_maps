#ifndef tp_maps_ImageShader_h
#define tp_maps_ImageShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing images.
class TP_MAPS_SHARED_EXPORT ImageShader: public Geometry3DShader
{
  friend class Map;
public:
  //################################################################################################
  ImageShader(const char* vertexShader=nullptr, const char* fragmentShader=nullptr);

  //################################################################################################
  ~ImageShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  //! Set the texture that will be draw, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

//  //################################################################################################
//  struct Vertex
//  {
//    glm::vec3 position;
//    glm::vec3 normal;
//    glm::vec2 texture;

//    Vertex()=default;
//    Vertex(const glm::vec3& position_,
//           const glm::vec3& normal_,
//           const glm::vec2& texture_):
//      position(position_),
//      normal(normal_),
//      texture(texture_)
//    {

//    }
//  };

//  //################################################################################################
//  struct VertexBuffer
//  {
//    //##############################################################################################
//    VertexBuffer(Map* map_, const Shader* shader_);

//    //##############################################################################################
//    ~VertexBuffer();

//    Map* map;
//    ShaderPointer shader;

//    //The Vertex Array Object
//    GLuint vaoID{0};

//    //The Index Buffer Object
//    GLuint iboID{0};

//    //The Vertex Buffer Object
//    GLuint vboID{0};

//    GLuint vertexCount{0};
//    TPGLsize indexCount{0};
//  };

//  //################################################################################################
//  VertexBuffer* generateVertexBuffer(Map* map,
//                                     const std::vector<GLushort>& indexes,
//                                     const std::vector<Vertex>& verts)const;

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
  static inline const tp_utils::StringID& name(){return imageShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
