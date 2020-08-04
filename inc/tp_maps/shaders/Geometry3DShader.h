#ifndef tp_maps_Geometry3DShader_h
#define tp_maps_Geometry3DShader_h

#include "tp_maps/Shader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! The base class for shaders drawing normal 3D geometry.
class TP_MAPS_SHARED_EXPORT Geometry3DShader: public Shader
{
public:
  //################################################################################################
  Geometry3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile);

  //################################################################################################
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 texture;

    Vertex()=default;
    Vertex(const glm::vec3& position_,
           const glm::vec3& normal_,
           const glm::vec3& tangent_,
           const glm::vec3& bitangent_,
           const glm::vec2& texture_):
      position(position_),
      normal(normal_),
      tangent(tangent_),
      bitangent(bitangent_),
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
    TPGLsize  indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<GLuint>& indexes,
                                     const std::vector<Vertex>& verts)const;
};

}

#endif
