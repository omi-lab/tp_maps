#include "tp_maps/shaders/PointSpriteShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource vertShaderStr{"/tp_maps/PointSpriteShader.vert"};
ShaderResource fragShaderStr{"/tp_maps/PointSpriteShader.frag"};

#ifdef TP_GLSL_PICKING_SUPPORTED
ShaderResource vertShaderStrPicking{"/tp_maps/PointSpriteShader.picking.vert"};
ShaderResource fragShaderStrPicking{"/tp_maps/PointSpriteShader.picking.frag"};
#endif

//There will be 4 of these generated for each PointSprite.
struct PointSprite_lt
{
  glm::vec4 color{};    //The color to multiply the texture by.
  glm::vec3 position{}; //The center coordinate of the point sprite.
  glm::vec3 offset{};   //The offset of each corner in relation to the position.
  glm::vec2 texture{};  //Texture coords for this corner.
};
}

//##################################################################################################
struct PointSpriteShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::PointSpriteShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint renderMatrixLoc{0};
  GLint renderScaleFactorLoc{0};

  GLint pickingMatrixLoc{0};
  GLint pickingScaleFactorLoc{0};
  GLint pickingIDLoc{0};

  //Chosen when use is called
  GLint matrixLoc{0};
  GLint scaleFactorLoc{0};

  //################################################################################################
  void draw(PointSpriteShader::VertexBuffer* vertexBuffer)
  {
    if(vertexBuffer->indexCount<3)
      return;

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(GL_TRIANGLES,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_INT,
                   nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(GL_TRIANGLES, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
PointSpriteShader::PointSpriteShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Shader(map, openGLProfile),
  d(new Private())
{
  //We compile 2 shaders one for picking and the other for normal rendering
  auto compileShader = [this](
      const char* vert,
      const char* frag,
      const char* shaderName,
      ShaderType shaderType,
      GLint& matrixL,
      GLint& scaleFactorL,
      GLint* pickingIDL)
  {
    compile(vert,
            frag,
            [](GLuint program)
    {
      glBindAttribLocation(program, 0, "inColor");
      glBindAttribLocation(program, 1, "inPosition");
      glBindAttribLocation(program, 2, "inOffset");
      glBindAttribLocation(program, 3, "inTexture");
    },
    [shaderName, &matrixL, &scaleFactorL, pickingIDL](GLuint program)
    {
      matrixL      = glGetUniformLocation(program, "matrix"    );
      scaleFactorL = glGetUniformLocation(program, "scaleFactor");

      if(pickingIDL)
        (*pickingIDL) = glGetUniformLocation(program, "pickingID");

      if(matrixL     <0)tpWarning() << shaderName << " matrixL: "      << matrixL;
      if(scaleFactorL<0)tpWarning() << shaderName << " scaleFactorL: " << scaleFactorL;
    },
    shaderType);
  };

  compileShader(vertShaderStr.data(openGLProfile), fragShaderStr.data(openGLProfile), "PointSpriteShader_render", ShaderType:: Render, d-> renderMatrixLoc, d-> renderScaleFactorLoc, nullptr);
#ifdef TP_GLSL_PICKING_SUPPORTED
  compileShader(vertShaderStrPicking.data(openGLProfile), fragShaderStrPicking.data(openGLProfile), "PointSpriteShader_picking", ShaderType::Picking, d->pickingMatrixLoc, d->pickingScaleFactorLoc, &d->pickingIDLoc);
#else
  //Point sprite picking is not implemented on this platform.
#endif
}

//##################################################################################################
PointSpriteShader::~PointSpriteShader()
{
  delete d;
}

//##################################################################################################
void PointSpriteShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html

  switch(shaderType)
  {
  case ShaderType::Light: [[fallthrough]];
  case ShaderType::Render:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);
    d->matrixLoc = d->renderMatrixLoc;
    d->scaleFactorLoc = d->renderScaleFactorLoc;
    break;

  case ShaderType::Picking:
    glDisable(GL_BLEND);
    d->matrixLoc = d->pickingMatrixLoc;
    d->scaleFactorLoc = d->pickingScaleFactorLoc;
    break;
  }

  Shader::use(shaderType);
}

//##################################################################################################
void PointSpriteShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void PointSpriteShader::setScreenSize(const glm::vec2& screenSize)
{
  glm::vec2 ss = 2.0f/screenSize;
  glUniform2fv(d->scaleFactorLoc, 1, glm::value_ptr(ss));
}

//##################################################################################################
void PointSpriteShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
PointSpriteShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
PointSpriteShader::VertexBuffer::~VertexBuffer()
{  
  if(!shader.shader())
    return;

  map->makeCurrent();

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  if(vaoID)
    tpDeleteVertexArrays(1, &vaoID);

  if(iboID)
    glDeleteBuffers(1, &iboID);
#endif

  if(vboID)
    glDeleteBuffers(1, &vboID);
}

//##################################################################################################
void PointSpriteShader::VertexBuffer::bindVBO() const
{
  glBindBuffer(GL_ARRAY_BUFFER, vboID);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), tpVoidLiteral( 0)); //vec4 color;
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), tpVoidLiteral(16)); //vec3 position;
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), tpVoidLiteral(28)); //vec3 offset;
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), tpVoidLiteral(40)); //vec2 texture;
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
#endif
}

//##################################################################################################
PointSpriteShader::VertexBuffer* PointSpriteShader::generateVertexBuffer(Map* map,
                                                                         const std::vector<PointSpriteShader::PointSprite>& pointSptrites,
                                                                         const std::vector<SpriteCoords>& coords)const
{
  auto* vertexBuffer = new VertexBuffer(map, this);

  if(pointSptrites.empty())
    return vertexBuffer;

  std::vector<GLuint> indexes;
  std::vector<PointSprite_lt> verts;

  const std::array<glm::vec3, 4> offsets =
  {
    {
      {-1.0f,-1.0f,0.0f},
      { 1.0f,-1.0f,0.0f},
      { 1.0f, 1.0f,0.0f},
      {-1.0f, 1.0f,0.0f}
    }
  };

  const std::array<glm::vec2, 4> textureCoords =
  {
    {
      {0.0f,0.0f},
      {1.0f,0.0f},
      {1.0f,1.0f},
      {0.0f,1.0f}
    }
  };

  {
    const PointSpriteShader::PointSprite* p = pointSptrites.data();
    const PointSpriteShader::PointSprite* pMax = p + pointSptrites.size();
    for(; p<pMax; p++)
    {
      indexes.push_back(GLuint(verts.size()+0));
      indexes.push_back(GLuint(verts.size()+1));
      indexes.push_back(GLuint(verts.size()+3));
      indexes.push_back(GLuint(verts.size()+1));
      indexes.push_back(GLuint(verts.size()+2));
      indexes.push_back(GLuint(verts.size()+3));

      const auto& texCoords = (p->spriteIndex<coords.size())?coords.at(p->spriteIndex).coords:textureCoords;

      for(size_t i=0; i<4; i++)
      {
        PointSprite_lt& ps = verts.emplace_back();
        ps.color    = p->color;
        ps.position = p->position;
        ps.offset   =(p->offset+offsets.at(i))*p->radius;
        ps.texture  = texCoords.at(i);
      }
    }
  }
  vertexBuffer->indexCount  = GLuint(indexes.size());

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
  vertexBuffer->vertexCount = GLuint(verts.size());

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, TPGLsize(indexes.size()*sizeof(GLuint)), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, TPGLsize(verts.size()*sizeof(PointSprite_lt)), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);
  vertexBuffer->bindVBO();
  tpBindVertexArray(0);
#else
  vertexBuffer->vertexCount = GLuint(indexes.size());
  std::vector<PointSprite_lt> indexedVerts;
  indexedVerts.reserve(indexes.size());
  for(auto index : indexes)
    indexedVerts.push_back(verts.at(size_t(index)));

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, TPGLsize(indexedVerts.size()*sizeof(PointSprite_lt)), indexedVerts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif

  return vertexBuffer;
}

//##################################################################################################
void PointSpriteShader::deleteVertexBuffer(PointSpriteShader::VertexBuffer* vertexBuffer)const
{
  delete vertexBuffer;
}

//##################################################################################################
void PointSpriteShader::drawPointSprites(VertexBuffer* vertexBuffer)
{
  d->draw(vertexBuffer);
}

//##################################################################################################
void PointSpriteShader::drawPointSpritesPicking(VertexBuffer* vertexBuffer, uint32_t pickingID)
{
#if !defined(TP_GLSL_PICKING_SUPPORTED) || defined(TP_EMSCRIPTEN)
  TP_UNUSED(vertexBuffer);
  TP_UNUSED(pickingID);
#elif defined(TP_IOS)
  glUniform1i(d->pickingIDLoc, GLint(pickingID));
  d->draw(vertexBuffer);
#else
  glUniform1ui(d->pickingIDLoc, pickingID);
  d->draw(vertexBuffer);
#endif
}

}
