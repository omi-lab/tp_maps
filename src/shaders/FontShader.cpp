#include "tp_maps/shaders/FontShader.h"
#include "tp_maps/FontRenderer.h"
#include "tp_maps/Font.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderString vertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//FontShader vertexShaderStr\n"
    "$TP_GLSL_IN_V$vec3 inVertex;\n"
    "$TP_GLSL_IN_V$vec3 inNormal;\n"
    "$TP_GLSL_IN_V$vec2 inTexture;\n"
    "uniform mat4 matrix;\n"
    "uniform vec4 color;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec4 multColor;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = matrix * vec4(inVertex, 1.0);\n"
    "  texCoordinate = inTexture;\n"
    "  multColor = color;\n"
    "}\n";

ShaderString fragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//FontShader fragmentShaderStr\n"
    "$TP_GLSL_IN_F$vec2 texCoordinate;\n"
    "$TP_GLSL_IN_F$vec4 multColor;\n"
    "uniform sampler2D textureSampler;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "void main()\n"
    "{\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = $TP_GLSL_TEXTURE$ (textureSampler, texCoordinate)*multColor;\n"
    "  if($TP_GLSL_GLFRAGCOLOR$.a < 0.01)\n"
    "    discard;\n"
    "}\n";

//##################################################################################################
struct Vertex_lt
{
  glm::vec3 position{};
  glm::vec3 normal{};
  glm::vec2 texture{};
};
}

//##################################################################################################
struct FontShader::Private
{
  GLint matrixLocation{0};
  GLint colorLocation{0};
};

//##################################################################################################
struct FontShader::PreparedString::Private
{
  TP_NONCOPYABLE(Private);

  Map* map;
  ShaderPointer shader;

  //The Vertex Array Object
  GLuint vaoID{0};

  //The Index Buffer Object
  GLuint iboID{0};

  //The Vertex Buffer Object
  GLuint vboID{0};

  GLuint vertexCount{0};
  GLuint  indexCount{0};

  bool regenerateBuffers{true};
  bool valid{false};

  //################################################################################################
  Private(Map* map_, const Shader* shader_):
    map(map_),
    shader(shader_)
  {

  }

  //################################################################################################
  ~Private()
  {
    freeBuffers();
  }

  //################################################################################################
  void freeBuffers()
  {
    if(!vaoID)
      return;

    map->makeCurrent();
    tpDeleteVertexArrays(1, &vaoID);
    glDeleteBuffers(1, &iboID);
    glDeleteBuffers(1, &vboID);

    vaoID      = 0;
    iboID      = 0;
    vboID      = 0;
    vertexCount= 0;
    indexCount = 0;
    valid = false;
  }
};

//##################################################################################################
FontShader::FontShader(const char* vertexShader, const char* fragmentShader):
  d(new Private())
{
  if(!vertexShader)
    vertexShader = vertexShaderStr.data();

  if(!fragmentShader)
    fragmentShader = fragmentShaderStr.data();

  compile(vertexShader,
          fragmentShader,
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
    glBindAttribLocation(program, 2, "inTexture");
  },
  [this](GLuint program)
  {
    d->matrixLocation = glGetUniformLocation(program, "matrix");
    d->colorLocation  = glGetUniformLocation(program, "color");
    const char* shaderName = "FontShader";
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->matrixLocation: " << d->matrixLocation;
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->colorLocation: " << d->colorLocation;
  });
}

//##################################################################################################
FontShader::~FontShader()
{
  delete d;
}

//##################################################################################################
void FontShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  Shader::use(shaderType);
}

//##################################################################################################
void FontShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void FontShader::setColor(const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
}

//##################################################################################################
void FontShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
void FontShader::drawPreparedString(PreparedString& preparedString)
{
  setTexture(preparedString.textureID());

  if(preparedString.d->regenerateBuffers)
  {
    preparedString.d->freeBuffers();
    preparedString.d->regenerateBuffers = false;

    const auto& fontGeometry = preparedString.fontGeometry();

    std::vector<GLushort> indexes;
    std::vector<Vertex_lt> verts;

    indexes.reserve(fontGeometry.glyphs.size()*6);
    verts.reserve(fontGeometry.glyphs.size()*4);

    for(const auto& glyph : fontGeometry.glyphs)
    {
      auto i = verts.size();

      indexes.push_back(GLushort(i+0));
      indexes.push_back(GLushort(i+1));
      indexes.push_back(GLushort(i+2));

      indexes.push_back(GLushort(i+0));
      indexes.push_back(GLushort(i+2));
      indexes.push_back(GLushort(i+3));

      for(size_t i=0; i<4; i++)
      {
        Vertex_lt& vert = verts.emplace_back();
        vert.position = {glyph.vertices.at(i), 0.0f};
        if(preparedString.config().topDown)
          vert.position.y = -vert.position.y;
        vert.texture = glyph.textureCoords.at(i);
        vert.normal = {0.0f, 0.0f, 1.0f};
      }
    }

    if(indexes.empty() || verts.empty())
      return;

    preparedString.d->vertexCount = GLuint(verts.size());
    preparedString.d->indexCount  = GLuint(indexes.size());

    glGenBuffers(1, &preparedString.d->iboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, preparedString.d->iboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, GLsizeiptr(indexes.size()*sizeof(GLushort)), indexes.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenBuffers(1, &preparedString.d->vboID);
    glBindBuffer(GL_ARRAY_BUFFER, preparedString.d->vboID);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(verts.size()*sizeof(Vertex_lt)), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    tpGenVertexArrays(1, &preparedString.d->vaoID);
    tpBindVertexArray(preparedString.d->vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, preparedString.d->vboID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_lt), tpVoidLiteral( 0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_lt), tpVoidLiteral(12));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_lt), tpVoidLiteral(24));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, preparedString.d->iboID);

    tpBindVertexArray(0);

    preparedString.d->valid = true;
  }

  if(preparedString.d->valid)
  {
    tpBindVertexArray(preparedString.d->vaoID);
    tpDrawElements(GL_TRIANGLES,
                   preparedString.d->indexCount,
                   GL_UNSIGNED_SHORT,
                   nullptr);
    tpBindVertexArray(0);
  }
}

//##################################################################################################
FontShader::PreparedString::PreparedString(const Shader* shader,
                                           FontRenderer* fontRenderer,
                                           const std::u16string& text,
                                           const PreparedStringConfig& config):
  tp_maps::PreparedString(fontRenderer, text, config),
  d(new Private(fontRenderer->map(), shader))
{

}

//##################################################################################################
FontShader::PreparedString::~PreparedString()
{
  delete d;
}

//##################################################################################################
void FontShader::PreparedString::invalidateBuffers()
{
  //Something bad has happened to the OpenGL context so we should just forget all buffers and start
  //from scratch.

  d->regenerateBuffers = true;
  d->valid = false;

  d->vaoID       = 0;
  d->iboID       = 0;
  d->vboID       = 0;
  d->vertexCount = 0;
  d->indexCount  = 0;

  tp_maps::PreparedString::invalidateBuffers();
}

//##################################################################################################
void FontShader::PreparedString::regenerateBuffers()
{
  //The OpenGL context is still valid but the data has changed so we need to free existing buffers
  //and regenerate.

  d->regenerateBuffers = true;
  d->valid = false;

  tp_maps::PreparedString::regenerateBuffers();
}

}
