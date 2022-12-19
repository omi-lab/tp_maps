#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/ImageShader.vert"}; return s;}
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/ImageShader.frag"}; return s;}
ShaderResource& frag3DShaderStr(){static ShaderResource s{"/tp_maps/Image3DShader.frag"}; return s;}
}

//##################################################################################################
struct ImageShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::ImageShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint matrixLocation{0};
  GLint colorLocation{0};

  //################################################################################################
  void draw(GLenum mode, ImageShader::VertexBuffer* vertexBuffer)
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
ImageShader::ImageShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const char* vertexShader, const char* fragmentShader):
  Geometry3DShader(map, openGLProfile),
  d(new Private())
{
  if(!vertexShader)
    vertexShader = vertShaderStr().data(openGLProfile, ShaderType::Render);

  if(!fragmentShader)
    fragmentShader = fragShaderStr().data(openGLProfile, ShaderType::Render);

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
    const char* shaderName = "ImageShader";
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->matrixLocation: " << d->matrixLocation;
  });
}

//##################################################################################################
ImageShader::~ImageShader()
{
  delete d;
}

//##################################################################################################
void ImageShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}

//##################################################################################################
void ImageShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
void ImageShader::setTexture(GLuint textureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureID);
}

//##################################################################################################
void ImageShader::setTexture3D(GLuint textureID, size_t level)
{
  TP_UNUSED(level);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, textureID);
}

//##################################################################################################
void ImageShader::draw(GLenum mode, VertexBuffer* vertexBuffer, const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void ImageShader::drawPicking(GLenum mode, VertexBuffer* vertexBuffer)
{
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void ImageShader::init(RenderInfo& renderInfo,
                       const Matrices& m,
                       const glm::mat4& modelToWorldMatrix)
{
  TP_UNUSED(renderInfo);

  use();
  setMatrix(m.vp * modelToWorldMatrix);
}

//##################################################################################################
void ImageShader::setMaterial(RenderInfo& renderInfo,
                              const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  setTexture(processedGeometry3D.rgbaTextureID);
}

//##################################################################################################
void ImageShader::setMaterialPicking(RenderInfo& renderInfo,
                                     const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  setTexture(processedGeometry3D.rgbaTextureID);
}

//##################################################################################################
void ImageShader::draw(RenderInfo& renderInfo,
                       const ProcessedGeometry3D& processedGeometry3D,
                       GLenum mode,
                       VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);

  draw(mode, vertexBuffer, {1.0f, 1.0f, 1.0f, 1.0f});
}

//##################################################################################################
void ImageShader::drawPicking(RenderInfo& renderInfo,
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
Image3DShader::Image3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, frag3DShaderStr().data(openGLProfile, ShaderType::Render))
{

}

}
