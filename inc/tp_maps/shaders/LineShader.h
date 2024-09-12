#ifndef tp_maps_LineShader_h
#define tp_maps_LineShader_h

#include "tp_maps/Shader.h"

#include "tp_utils/RefCount.h"

#include "glm/glm.hpp" // IWYU pragma: keep

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing lines.
class TP_MAPS_EXPORT LineShader: public Shader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return lineShaderSID();}

  //################################################################################################
  LineShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~LineShader() override;

  //################################################################################################
  const std::string& vertexShaderStr(ShaderType shaderType) override;

  //################################################################################################
  const std::string& fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void bindLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void init() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType) override;

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
    TP_REF_COUNT_OBJECTS("LineShader::VertexBuffer");

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
    GLuint indexCount{0};
  };

  //################################################################################################
  VertexBuffer* generateVertexBuffer(Map* map, const std::vector<glm::vec3>& vertices) const;

  //################################################################################################
  //! Call this to draw the lines
  /*!
  \param mode One of GL_LINES, GL_LINE_LOOP, GL_LINE_STRIP
  \param vertices The points that make up the line.
  */
  void drawLines(GLenum mode, LineShader::VertexBuffer* vertexBuffer);
};

}

#endif
