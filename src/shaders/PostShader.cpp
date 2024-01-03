#include "tp_maps/shaders/PostShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct PostShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::detail::PostShaderPrivate::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint textureLocation {-1};
  GLint depthLocation   {-1};
  GLint normalsLocation {-1};
  GLint specularLocation{-1};

  GLint projectionMatrixLocation   {-1};
  GLint invProjectionMatrixLocation{-1};

  GLint pixelSizeLocation{-1};
};

//##################################################################################################
PostShader::PostShader(Map* map, tp_maps::ShaderProfile shaderProfile):
  FullScreenShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
PostShader::~PostShader()
{
  delete d;
}

//##################################################################################################
void PostShader::setReadFBO(const FBO& readFBO)
{
  if(d->textureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, readFBO.textureID);
    glUniform1i(d->textureLocation, 0);
  }

  if(d->depthLocation>=0)
  {
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, readFBO.depthID);
    glUniform1i(d->depthLocation, 1);
  }

  if(d->normalsLocation>=0)
  {
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, readFBO.normalsID);
    glUniform1i(d->normalsLocation, 2);
  }

  if(d->specularLocation>=0)
  {
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, readFBO.specularID);
    glUniform1i(d->specularLocation, 3);
  }
}

//##################################################################################################
void PostShader::setFBOSourceTexture( const GLuint sourceTextureID )
{
  if(d->textureLocation >= 0)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, sourceTextureID);
    glUniform1i(d->textureLocation, 0);
  }
}

//##################################################################################################
void PostShader::setProjectionMatrix(const glm::mat4& projectionMatrix)
{
  if(d->projectionMatrixLocation>=0)
    glUniformMatrix4fv(d->projectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

  if(d->invProjectionMatrixLocation>=0)
  {
    glm::mat4 invProjectionMatrix = glm::inverse(projectionMatrix);
    glUniformMatrix4fv(d->invProjectionMatrixLocation, 1, GL_FALSE, glm::value_ptr(invProjectionMatrix));
  }

  if(d->pixelSizeLocation>=0)
    glUniform2f(d->pixelSizeLocation, 1.0f/float(map()->width()), 1.0f/float(map()->height()));
}

//##################################################################################################
void PostShader::getLocations(GLuint program, ShaderType shaderType)
{
  FullScreenShader::getLocations(program, shaderType);

  getLocation(program, d->textureLocation            , "textureSampler"     );
  getLocation(program, d->depthLocation              , "depthSampler"       );
  getLocation(program, d->normalsLocation            , "normalsSampler"     );
  getLocation(program, d->specularLocation           , "specularSampler"    );

  getLocation(program, d->projectionMatrixLocation   , "projectionMatrix"   );
  getLocation(program, d->invProjectionMatrixLocation, "invProjectionMatrix");

  getLocation(program, d->pixelSizeLocation          , "pixelSize"          );
}

//##################################################################################################
void PostShader::init()
{
  if(map()->extendedFBO() == ExtendedFBO::Yes)
    compile(ShaderType::RenderExtendedFBO);
  else
    compile(ShaderType::Render);
}

}
