#include "tp_maps/shaders/FontShader.h"
#include "tp_maps/FontRenderer.h"
#include "tp_maps/Font.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"

#include "glm/gtc/type_ptr.hpp"

//Note: GL

namespace tp_maps
{

namespace
{
//##################################################################################################
struct Vertex_lt
{
  glm::vec3 position{};
  glm::vec2 texture{};
};
}

//##################################################################################################
struct FontShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FontShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  GLint colorLocation{0};
};

//##################################################################################################
struct FontShader::PreparedString::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FontShader::PreparedString::Private");
  TP_NONCOPYABLE(Private);

  Map* map;

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
  Private(Map* map_):
    map(map_)
  {

  }

  //################################################################################################
  ~Private()
  {
    freeBuffers();
  }

  //################################################################################################
  void bindVBO()
  {
    TP_FUNCTION_TIME("FontShader::PreparedString::Private::bindVBO");

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex_lt), tpVoidLiteral( 0));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex_lt), tpVoidLiteral(12));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboID);
  }

  //################################################################################################
  void freeBuffers()
  {
    if(!vaoID)
      return;

    map->makeCurrent();

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpDeleteVertexArrays(1, &vaoID);
#endif

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
FontShader::FontShader(Map* map, tp_maps::ShaderProfile shaderProfile):
  Shader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
FontShader::~FontShader()
{
  delete d;
}

//##################################################################################################
void FontShader::setMatrix(const glm::mat4& matrix)
{
  TP_FUNCTION_TIME("FontShader::setMatrix");
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void FontShader::setColor(const glm::vec4& color)
{
  TP_FUNCTION_TIME("FontShader::setColor");
  glUniform4fv(d->colorLocation, 1, &color.x);
}

//##################################################################################################
void FontShader::setTexture(GLuint textureID)
{
  TP_FUNCTION_TIME("FontShader::setTexture");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
void FontShader::drawPreparedString(PreparedString& preparedString)
{
  TP_FUNCTION_TIME("FontShader::drawPreparedString");

  setTexture(preparedString.textureID());

  if(preparedString.d->regenerateBuffers)
  {
    preparedString.d->freeBuffers();
    preparedString.d->regenerateBuffers = false;

    const auto& fontGeometry = preparedString.fontGeometry();

    std::vector<GLushort> indexes;
    std::vector<Vertex_lt> verts;

    {
      TP_FUNCTION_TIME("FontShader::drawPreparedString generate indexes");

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
        }
      }

      if(indexes.empty() || verts.empty())
        return;

      preparedString.d->indexCount  = GLuint(indexes.size());
    }

    {
      TP_FUNCTION_TIME("FontShader::drawPreparedString create arrays");

#ifdef TP_VERTEX_ARRAYS_SUPPORTED
      preparedString.d->vertexCount = GLuint(verts.size());

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
      preparedString.d->bindVBO();
      tpBindVertexArray(0);
#else
      preparedString.d->vertexCount = GLuint(indexes.size());

      std::vector<Vertex_lt> indexedVerts;
      indexedVerts.reserve(indexes.size());
      for(auto index : indexes)
        indexedVerts.push_back(verts.at(size_t(index)));

      glGenBuffers(1, &preparedString.d->vboID);
      glBindBuffer(GL_ARRAY_BUFFER, preparedString.d->vboID);
      glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(indexedVerts.size()*sizeof(Vertex_lt)), indexedVerts.data(), GL_STATIC_DRAW);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
#endif
    }

    preparedString.d->valid = true;
  }

  if(preparedString.d->valid)
  {
    TP_FUNCTION_TIME("FontShader::drawPreparedString draw");
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(preparedString.d->vaoID);
    tpDrawElements(GL_TRIANGLES,
                   preparedString.d->indexCount,
                   GL_UNSIGNED_SHORT,
                   nullptr);
    tpBindVertexArray(0);
#else
    preparedString.d->bindVBO();
    glDrawArrays(GL_TRIANGLES, 0, preparedString.d->indexCount);
#endif
  }
}

//##################################################################################################
void FontShader::use(ShaderType shaderType)
{
  TP_FUNCTION_TIME("FontShader::use");

  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  Shader::use(shaderType);
}

//##################################################################################################
const char* FontShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/FontShader.vert"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
const char* FontShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/FontShader.frag"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
void FontShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertex");
  glBindAttribLocation(program, 1, "inTexture");
}

//##################################################################################################
void FontShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  d->matrixLocation = glGetUniformLocation(program, "matrix");
  d->colorLocation  = glGetUniformLocation(program, "color");

  if(d->matrixLocation<0)
    tpWarning() << "FontShader d->matrixLocation: " << d->matrixLocation;

  if(d->colorLocation<0)
    tpWarning() << "FontShader d->colorLocation: " << d->colorLocation;
}

//##################################################################################################
void FontShader::init()
{
  if(map()->extendedFBO() == ExtendedFBO::Yes)
    compile(ShaderType::RenderExtendedFBO);
  else
    compile(ShaderType::Render);
}

//##################################################################################################
FontShader::PreparedString::PreparedString(FontRenderer* fontRenderer,
                                           const std::u16string& text,
                                           const PreparedStringConfig& config):
  tp_maps::PreparedString(fontRenderer, text, config),
  d(new Private(fontRenderer->map()))
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
  TP_FUNCTION_TIME("FontShader::invalidateBuffers");

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
