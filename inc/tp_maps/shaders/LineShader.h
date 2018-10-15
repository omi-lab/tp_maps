#ifndef tp_maps_LineShader_h
#define tp_maps_LineShader_h

#include "tp_maps/Shader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! The base class for shaders.
/*!
This allows the map to cache shaders.
*/
class TP_MAPS_SHARED_EXPORT LineShader: public Shader
{
  friend class Map;
public:
  //################################################################################################
  LineShader();

  //################################################################################################
  ~LineShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the lines
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  void setLineWidth(float lineWidth);

  //################################################################################################
  void setColor(const glm::vec4& color);

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
    GLuint indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map, const std::vector<glm::vec3>& vertices)const;

  //################################################################################################
  //! Call this to draw the lines
  /*!
  \param mode One of GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP
  \param vertices The points that make up the line.
  */
  void drawLines(GLenum mode, LineShader::VertexBuffer* vertexBuffer);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return lineShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
