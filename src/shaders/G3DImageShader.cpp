#include "tp_maps/shaders/G3DImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct G3DImageShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::G3DImageShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, G3DImageShader::VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_INT,
                   nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
G3DImageShader::G3DImageShader(Map* map, ShaderProfile shaderProfile):
  Geometry3DShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
G3DImageShader::~G3DImageShader()
{
  delete d;
}

//##################################################################################################
void G3DImageShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void G3DImageShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
void G3DImageShader::draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DImageShader::drawPicking(GLenum mode, VertexBuffer* vertexBuffer)
{
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DImageShader::initPass(RenderInfo& renderInfo,
                              const Matrices& m,
                              const glm::mat4& modelToWorldMatrix)
{
  use(renderInfo.shaderType());
  setMatrix(m.vp * modelToWorldMatrix);
}

//##################################################################################################
void G3DImageShader::setMaterial(RenderInfo& renderInfo,
                                 const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  setTexture(processedGeometry3D.rgbaTextureID);
}

//##################################################################################################
void G3DImageShader::setMaterialPicking(RenderInfo& renderInfo,
                                        const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  setTexture(processedGeometry3D.rgbaTextureID);
}

//##################################################################################################
void G3DImageShader::draw(RenderInfo& renderInfo,
                          const ProcessedGeometry3D& processedGeometry3D,
                          GLenum mode,
                          VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);

  draw(mode, vertexBuffer, {1.0f, 1.0f, 1.0f, 1.0f});
}

//##################################################################################################
void G3DImageShader::drawPicking(RenderInfo& renderInfo,
                                 const ProcessedGeometry3D& processedGeometry3D,
                                 GLenum mode,
                                 VertexBuffer* vertexBuffer,
                                 const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
  TP_UNUSED(pickingID);

  drawPicking(mode, vertexBuffer);
}

//##################################################################################################
void G3DImageShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Geometry3DShader::use(shaderType);
}

//##################################################################################################
const char* G3DImageShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DImageShader.vert"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
const char* G3DImageShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DImageShader.frag"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
void G3DImageShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertex");
  glBindAttribLocation(program, 1, "inTBNq");
  glBindAttribLocation(program, 2, "inTexture");
}

//##################################################################################################
void G3DImageShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  d->matrixLocation = glGetUniformLocation(program, "matrix");
  d->colorLocation  = glGetUniformLocation(program, "color");

  if(d->matrixLocation<0)
    tpWarning() << "G3DImageShader d->matrixLocation: " << d->matrixLocation;

  if(d->colorLocation<0)
    tpWarning() << "G3DImageShader d->colorLocation: " << d->colorLocation;
}

}
