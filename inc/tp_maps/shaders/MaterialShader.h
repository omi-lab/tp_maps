#ifndef tp_maps_MaterialShader_h
#define tp_maps_MaterialShader_h

#include "tp_maps/Shader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! The base class for shaders.
/*!
This allows the map to cache shaders.
*/
class TP_MAPS_SHARED_EXPORT MaterialShader: public Shader
{
  friend class Map;
public:
  //################################################################################################
  MaterialShader();

  //################################################################################################
  ~MaterialShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  struct Material
  {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    float alpha;
  };

  //################################################################################################
  struct Light
  {
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 ambient{0.4f, 0.4f, 0.4f};
    glm::vec3 diffuse{0.8f, 0.8f, 0.8f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
  };

  //################################################################################################
  //! Call this to set the material before drawing the geometry
  void setMaterial(const Material& material);

  //################################################################################################
  //! Call this to set the light before drawing the geometry
  void setLight(const Light& light);

  //################################################################################################
  //! Call this to set the camera matrix before drawing the geometry
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;

    Vertex(){}
    Vertex(const glm::vec3& position_,
           const glm::vec3& normal_):
      position(position_),
      normal(normal_)
    {

    }
  };

  //################################################################################################
  struct VertexBuffer
  {
    //##############################################################################################
    VertexBuffer(Map* map_, const Shader* shader_);

    //##############################################################################################
    ~VertexBuffer();

    Map* map;
    ShaderPointer shader;

    //The Vertex Array Object
    GLuint vaoID{0};

    //The Index Buffer Object
    GLuint iboID{0};

    //The Vertex Buffer Object
    GLuint vboID{0};

    GLuint vertexCount{0};
    GLsizei indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<GLushort>& indexes,
                                     const std::vector<Vertex>& verts)const;

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param vertices The points that make up the line.
  */
  void draw(GLenum mode, VertexBuffer* vertexBuffer);

  //################################################################################################
  //! Call this to draw the image for picking
  /*!
  \param vertices The points that make up the line.
  */
  void drawPicking(GLenum mode,
                   VertexBuffer* vertexBuffer,
                   const glm::vec4& pickingID);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return materialShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
