#include "tp_maps/shaders/G3DFlatColorShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct G3DFlatColorShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::G3DFlatColorShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, Geometry3DShader::VertexBuffer* vertexBuffer)
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
G3DFlatColorShader::G3DFlatColorShader(Map* map, ShaderProfile shaderProfile):
  Geometry3DShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
G3DFlatColorShader::~G3DFlatColorShader()
{
  delete d;
}

//##################################################################################################
void G3DFlatColorShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void G3DFlatColorShader::draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
bool G3DFlatColorShader::initPass(RenderInfo& renderInfo,
                                  const Matrices& m,
                                  const glm::mat4& modelToWorldMatrix)
{
  use(renderInfo.shaderType());
  setMatrix(m.vp * modelToWorldMatrix);

  return true;
}

//##################################################################################################
void G3DFlatColorShader::setMaterial(RenderInfo& renderInfo,
                                     const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void G3DFlatColorShader::setMaterialPicking(RenderInfo& renderInfo,
                                            const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void G3DFlatColorShader::draw(RenderInfo& renderInfo,
                              const ProcessedGeometry3D& processedGeometry3D,
                              GLenum mode,
                              VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  draw(mode, vertexBuffer, processedGeometry3D.material.rgba());
}

//##################################################################################################
void G3DFlatColorShader::drawPicking(RenderInfo& renderInfo,
                                     const ProcessedGeometry3D& processedGeometry3D,
                                     GLenum mode,
                                     VertexBuffer* vertexBuffer,
                                     const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
  draw(mode, vertexBuffer, pickingID);
}

//##################################################################################################
void G3DFlatColorShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Geometry3DShader::use(shaderType);
}

//##################################################################################################
const char* G3DFlatColorShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DFlatColorShader.vert"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
const char* G3DFlatColorShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DFlatColorShader.frag"};
  return s.data(shaderProfile(), shaderType);
}

//##################################################################################################
void G3DFlatColorShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertex");
  glBindAttribLocation(program, 1, "inTBNq");
  glBindAttribLocation(program, 2, "inTexture");
}

//##################################################################################################
void G3DFlatColorShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  d->matrixLocation = glGetUniformLocation(program, "matrix");
  d->colorLocation  = glGetUniformLocation(program, "color");

  if(d->matrixLocation<0)
    tpWarning() << "G3DFlatColorShader d->matrixLocation: " << d->matrixLocation;

  if(d->colorLocation<0)
    tpWarning() << "G3DFlatColorShader d->colorLocation: " << d->colorLocation;
}

}
