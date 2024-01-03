#ifndef tp_maps_PointSpriteShader_h
#define tp_maps_PointSpriteShader_h

#include "tp_maps/Shader.h"
#include "tp_maps/SpriteTexture.h"

#include "tp_utils/RefCount.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing point sprites.
class TP_MAPS_EXPORT PointSpriteShader: public Shader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return pointSpriteShaderSID();}

  //################################################################################################
  PointSpriteShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~PointSpriteShader() override;

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

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the lines
  void setMatrix(const glm::mat4& matrix);

  //################################################################################################
  void setScreenSize(const glm::vec2& screenSize);

  //################################################################################################
  //! Set the texture that will be draw, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  struct PointSprite
  {
    glm::vec4 color;
    glm::vec3 position;
    glm::vec3 offset{0.0f, 0.0f, 0.0f};
    size_t spriteIndex{0};
    float radius{1.0f};

    //##############################################################################################
    PointSprite() = default;

    //##############################################################################################
    PointSprite(const glm::vec4& color_, const glm::vec3& position_, size_t spriteIndex_, float radius_):
      color(color_),
      position(position_),
      spriteIndex(spriteIndex_),
      radius(radius_)
    {

    }
  };

  //################################################################################################
  struct VertexBuffer
  {
    TP_REF_COUNT_OBJECTS("PointSpriteShader::VertexBuffer");

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
  VertexBuffer* generateVertexBuffer(Map* map,
                                     const std::vector<PointSprite>& pointSptrites,
                                     const std::vector<SpriteCoords>& coords) const;

  //################################################################################################
  void deleteVertexBuffer(VertexBuffer* vertexBuffer) const;

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param vertices The points that make up the line.
  */
  void drawPointSprites(VertexBuffer* vertexBuffer);

  //################################################################################################
  //! Call this to draw the image for picking
  /*!
  \param vertices The points that make up the line.
  */
  void drawPointSpritesPicking(VertexBuffer* vertexBuffer, uint32_t pickingID);
};

}

#endif
