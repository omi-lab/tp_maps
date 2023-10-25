#include "tp_maps/shaders/G3DXYZShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct G3DXYZShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::G3DXYZShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint mLocation{0};
  GLint mvpLocation{0};
  GLint uvMatrixLocation{0};

  GLint rgbaTextureLocation{0};

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);

    glDepthFunc(GL_LESS);
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
G3DXYZShader::G3DXYZShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Geometry3DShader(map, openGLProfile),
  d(new Private())
{

}

//##################################################################################################
G3DXYZShader::~G3DXYZShader()
{
  delete d;
}

//##################################################################################################
void G3DXYZShader::setMatrix(const glm::mat4& m, const glm::mat4& mvp)
{
  glUniformMatrix4fv(d->mLocation, 1, GL_FALSE, glm::value_ptr(m));
  glUniformMatrix4fv(d->mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
}

//##################################################################################################
void G3DXYZShader::initPass(RenderInfo& renderInfo,
                        const Matrices& m,
                        const glm::mat4& modelToWorldMatrix)
{
  use(renderInfo.shaderType());
  setMatrix(modelToWorldMatrix, m.vp * modelToWorldMatrix);
}

//##################################################################################################
void G3DXYZShader::setMaterial(RenderInfo& renderInfo, const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  const auto& material = processedGeometry3D.alternativeMaterial->material;
  glm::mat3 uvMatrix = processedGeometry3D.uvMatrix * material.uvTransformation.uvMatrix();

  glUniformMatrix3fv(d->uvMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, processedGeometry3D.alternativeMaterial->rgbaTextureID);
  glUniform1i(d->rgbaTextureLocation, 0);
}

//##################################################################################################
void G3DXYZShader::setMaterialPicking(RenderInfo& renderInfo,
                                      const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void G3DXYZShader::draw(RenderInfo& renderInfo,
                        const ProcessedGeometry3D& processedGeometry3D,
                        GLenum mode,
                        VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DXYZShader::drawPicking(RenderInfo& renderInfo,
                               const ProcessedGeometry3D& processedGeometry3D,
                               GLenum mode,
                               VertexBuffer* vertexBuffer,
                               const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
  TP_UNUSED(pickingID);

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DXYZShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}
//##################################################################################################
const char* G3DXYZShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DXYZShader.vert"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
const char* G3DXYZShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DXYZShader.frag"};
  return s.data(openGLProfile(), shaderType);
}

//##################################################################################################
void G3DXYZShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertex");
  glBindAttribLocation(program, 2, "inTexture");
}

//##################################################################################################
void G3DXYZShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  auto loc = [](auto program, const auto* name){return glGetUniformLocation(program, name);};

  d->mLocation = loc(program, "m");
  if(d->mLocation<0)
    tpWarning() << "G3DXYZShader d->mLocation: " << d->mLocation;

  d->mvpLocation  = loc(program, "mvp");
  if(d->mvpLocation<0)
    tpWarning() << "G3DXYZShader d->mvpLocation: " << d->mvpLocation;

  d->uvMatrixLocation  = loc(program, "uvMatrix");
  if(d->uvMatrixLocation<0)
    tpWarning() << "G3DXYZShader d->uvMatrixLocation: " << d->uvMatrixLocation;

  d->rgbaTextureLocation = loc(program, "rgbaTexture");
  if(d->rgbaTextureLocation<0)
    tpWarning() << "G3DXYZShader d->rgbaTextureLocation: " << d->rgbaTextureLocation;
}

}
