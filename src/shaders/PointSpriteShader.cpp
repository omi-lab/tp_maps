#include "tp_maps/shaders/PointSpriteShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderString renderVertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//PointSpriteShader renderVertexShaderStr\n"
    "$TP_GLSL_IN_V$vec4 inColor;\n"
    "$TP_GLSL_IN_V$vec3 inPosition;\n"
    "$TP_GLSL_IN_V$vec3 inOffset;\n"
    "$TP_GLSL_IN_V$vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    "uniform vec2 scaleFactor;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec4 color;\n"
    "$TP_GLSL_OUT_V$float clip;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = (matrix * vec4(inPosition, 1.0));\n"
    "  clip = (gl_Position.z<-0.9999)?0.0:1.0;\n"
    "  gl_Position += vec4((inOffset.x*scaleFactor.x)*gl_Position.w, (inOffset.y*scaleFactor.y)*gl_Position.w, 0.0, 0.0);\n"
    "  texCoordinate = inTexture;\n"
    "  color = inColor;\n"
    "}\n";

ShaderString renderFragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//PointSpriteShader renderFragmentShaderStr\n"
    "uniform sampler2D textureSampler;\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "$TP_GLSL_IN_F$vec4 color;\n"
    "$TP_GLSL_IN_F$float clip;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = $TP_GLSL_TEXTURE$(textureSampler, texCoordinate) * color;\n"
    "  if($TP_GLSL_GLFRAGCOLOR$.a < 0.001 || clip<0.1)\n"
    "    discard;\n"
    "}\n";

#ifdef TP_GLSL_PICKING
ShaderString pickingVertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//PointSpriteShader pickingVertexShaderStr\n"
    "$TP_GLSL_IN_V$vec4 inColor;\n"
    "$TP_GLSL_IN_V$vec3 inPosition;\n"
    "$TP_GLSL_IN_V$vec3 inOffset;\n"
    "$TP_GLSL_IN_V$vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    "uniform vec2 scaleFactor;\n"
    "uniform uint pickingID;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec4 picking;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = (matrix * vec4(inPosition, 1.0));\n"
    "  gl_Position = vec4(gl_Position.xyz * (1.0/gl_Position.w), 1.0) + vec4(inOffset.x*scaleFactor.x, inOffset.y*scaleFactor.y, 0.0, 0.0);\n"
    "  texCoordinate = inTexture;\n"
    "  uint id = pickingID + (uint(gl_VertexID)/4u);\n"
    "  uint r = (id & 0x000000FFu) >>  0u;\n"
    "  uint g = (id & 0x0000FF00u) >>  8u;\n"
    "  uint b = (id & 0x00FF0000u) >> 16u;\n"
    "  picking = vec4(r,g,b,255.0)/255.0;\n"
    "}\n";

ShaderString pickingFragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//PointSpriteShader pickingFragmentShaderStr\n"
    "uniform sampler2D textureSampler;\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "$TP_GLSL_IN_F$vec4 picking;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = picking;\n"
    "  if($TP_GLSL_TEXTURE$(textureSampler, texCoordinate).a < 0.001)\n"
    "    discard;\n"
    "}\n";
#endif
}

//##################################################################################################
struct PointSpriteShader::Private
{
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

    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(GL_TRIANGLES,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_INT,
                   nullptr);
    tpBindVertexArray(0);
  }
};

//##################################################################################################
PointSpriteShader::PointSpriteShader():
  Shader(),
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

  compileShader( renderVertexShaderStr.data(),  renderFragmentShaderStr.data(),  "PointSpriteShader_render", ShaderType:: Render, d-> renderMatrixLoc, d-> renderScaleFactorLoc, nullptr);
#ifdef TP_GLSL_PICKING
  compileShader(pickingVertexShaderStr.data(), pickingFragmentShaderStr.data(), "PointSpriteShader_picking", ShaderType::Picking, d->pickingMatrixLoc, d->pickingScaleFactorLoc, &d->pickingIDLoc);
#else
#  warning fix point sprite picking on this platform.
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
  if(!vaoID || !shader.shader())
    return;

  map->makeCurrent();
  tpDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &iboID);
  glDeleteBuffers(1, &vboID);
}

//##################################################################################################
PointSpriteShader::VertexBuffer* PointSpriteShader::generateVertexBuffer(Map* map,
                                                                         const std::vector<PointSpriteShader::PointSprite>& pointSptrites,
                                                                         const std::vector<SpriteCoords>& coords)const
{
  VertexBuffer* vertexBuffer = new VertexBuffer(map, this);

  if(pointSptrites.empty())
    return vertexBuffer;

  //There will be 4 of these generated for each PointSprite.
  struct PointSprite_lt
  {
    glm::vec4 color;    //The color to multiply th etexture by.
    glm::vec3 position; //The center coordinate of the point sprite.
    glm::vec3 offset;   //The offset of each corner in relation to the position.
    glm::vec2 texture;  //Texture coords for this corner.
  };

  std::vector<GLuint> indexes;
  std::vector<PointSprite_lt> verts;

  const glm::vec3 offsets[4] =
  {
    {-1.0f,-1.0f,0.0f},
    { 1.0f,-1.0f,0.0f},
    { 1.0f, 1.0f,0.0f},
    {-1.0f, 1.0f,0.0f}
  };

  const glm::vec2 textureCoords[4] =
  {
    {0.0f,0.0f},
    {1.0f,0.0f},
    {1.0f,1.0f},
    {0.0f,1.0f}
  };

  {
    const PointSpriteShader::PointSprite* p = pointSptrites.data();
    const PointSpriteShader::PointSprite* pMax = p + pointSptrites.size();
    for(; p<pMax; p++)
    {
      indexes.push_back(verts.size()+0);
      indexes.push_back(verts.size()+1);
      indexes.push_back(verts.size()+3);
      indexes.push_back(verts.size()+1);
      indexes.push_back(verts.size()+2);
      indexes.push_back(verts.size()+3);

      const auto& texCoords = (p->spriteIndex>=0 && p->spriteIndex<int(coords.size()))?coords.at(p->spriteIndex).coords:textureCoords;

      for(int i=0; i<4; i++)
      {
        PointSprite_lt ps;
        ps.color    = p->color;
        ps.position = p->position;
        ps.offset   =(p->offset+offsets[i])*p->radius;
        ps.texture  = texCoords[i];
        verts.push_back(ps);
      }
    }
  }

  vertexBuffer->vertexCount = verts.size();
  vertexBuffer->indexCount  = indexes.size();

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size()*sizeof(GLuint), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(PointSprite_lt), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), (void*)(0));                //vec4 color;
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), (void*)(sizeof(float)*4));  //vec3 position;
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), (void*)(sizeof(float)*7));  //vec3 offset;
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(PointSprite_lt), (void*)(sizeof(float)*10)); //vec2 texture;
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);
  glEnableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);

  tpBindVertexArray(0);

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
#ifdef TDP_IOS
  glUniform1i(d->pickingIDLoc, GLint(pickingID));
  d->draw(vertexBuffer);
#elif defined(TDP_EMSCRIPTEN)

#else
  glUniform1ui(d->pickingIDLoc, pickingID);
  d->draw(vertexBuffer);
#endif
}

}
