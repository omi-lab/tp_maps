#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource& vertShaderStr(){static ShaderResource s{"/tp_maps/ImageShader.vert"}; return s;}
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/ImageShader.frag"}; return s;}
ShaderResource& frag3DShaderStr(){static ShaderResource s{"/tp_maps/DepthImage3DShader.frag"}; return s;}
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
    vertexShader = vertShaderStr().data(openGLProfile);

  if(!fragmentShader)
    fragmentShader = fragShaderStr().data(openGLProfile);

  compile(vertexShader,
          fragmentShader,
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
    glBindAttribLocation(program, 2, "inTangent");
    glBindAttribLocation(program, 3, "inBitangent");
    glBindAttribLocation(program, 4, "inTexture");
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
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

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
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, textureID);
}

//##################################################################################################
void ImageShader::draw(GLenum mode,
                            VertexBuffer* vertexBuffer,
                            const glm::vec4& color)
{
  glUniform4fv(d->colorLocation, 1, &color.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void ImageShader::drawPicking(GLenum mode,
                                   VertexBuffer* vertexBuffer,
                                   const glm::vec4& pickingID)
{
  TP_UNUSED(pickingID);
  glDisable(GL_BLEND);
  d->draw(mode, vertexBuffer);
}
//##################################################################################################
Image3DShader::Image3DShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  ImageShader(map, openGLProfile, nullptr, frag3DShaderStr().data(openGLProfile))
{

}

}
