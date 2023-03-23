#include "tp_maps/shaders/XYZShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/XYZShader.vert"}; return s;}
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/XYZShader.frag"}; return s;}
}

//##################################################################################################
struct XYZShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::XYZShader::Private");
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
XYZShader::XYZShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Geometry3DShader(map, openGLProfile),
  d(new Private())
{
  const char* vertexShader = vertShaderStr().data(openGLProfile, ShaderType::Render);
  const char* fragmentShader = fragShaderStr().data(openGLProfile, ShaderType::Render);

  compile(vertexShader,
          fragmentShader,
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 2, "inTexture");
  },
  [this](GLuint program)
  {
    auto loc = [](auto program, const auto* name){return glGetUniformLocation(program, name);};

    d->mLocation = loc(program, "m");
    if(d->mLocation<0)
      tpWarning() << "XYZShader d->mLocation: " << d->mLocation;

    d->mvpLocation  = loc(program, "mvp");
    if(d->mvpLocation<0)
      tpWarning() << "XYZShader d->mvpLocation: " << d->mvpLocation;

    d->uvMatrixLocation  = loc(program, "uvMatrix");
    if(d->uvMatrixLocation<0)
      tpWarning() << "XYZShader d->uvMatrixLocation: " << d->uvMatrixLocation;

    d->rgbaTextureLocation = loc(program, "rgbaTexture");
    if(d->rgbaTextureLocation<0)
      tpWarning() << "XYZShader d->rgbaTextureLocation: " << d->rgbaTextureLocation;
  });
}

//##################################################################################################
XYZShader::~XYZShader()
{
  delete d;
}

//##################################################################################################
void XYZShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}

//##################################################################################################
void XYZShader::setMatrix(const glm::mat4& m, const glm::mat4& mvp)
{
  glUniformMatrix4fv(d->mLocation, 1, GL_FALSE, glm::value_ptr(m));
  glUniformMatrix4fv(d->mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
}

//##################################################################################################
void XYZShader::init(RenderInfo& renderInfo,
                       const Matrices& m,
                       const glm::mat4& modelToWorldMatrix)
{
  TP_UNUSED(renderInfo);

  use();
  setMatrix(modelToWorldMatrix, m.vp * modelToWorldMatrix);
}

//##################################################################################################
void XYZShader::setMaterial(RenderInfo& renderInfo, const ProcessedGeometry3D& processedGeometry3D)
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
void XYZShader::setMaterialPicking(RenderInfo& renderInfo,
                                     const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void XYZShader::draw(RenderInfo& renderInfo,
                       const ProcessedGeometry3D& processedGeometry3D,
                       GLenum mode,
                       VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void XYZShader::drawPicking(RenderInfo& renderInfo,
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

}
