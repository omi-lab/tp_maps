#include "tp_maps/shaders/PostDoFMergeShader.h"

namespace tp_maps
{

//##################################################################################################
struct PostDoFMergeShader::Private
{
  GLint downsampledTextureLocation{0};
  GLint focusTextureLocation{0};
  GLint downsampledFocusTextureLocation{0};
};

//##################################################################################################
PostDoFMergeShader::PostDoFMergeShader(Map* map,
                                       tp_maps::ShaderProfile shaderProfile,
                                       const PostDoFParameters& parameters):
  PostDoFBaseShader(map, shaderProfile, parameters),
  d(new Private())
{

}

//##################################################################################################
PostDoFMergeShader::~PostDoFMergeShader()
{
  delete d;
}

//##################################################################################################
void PostDoFMergeShader::setDownsampledTexture(const GLuint downsampledTextureID)
{
  if(d->downsampledTextureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, downsampledTextureID);
    glUniform1i(d->downsampledTextureLocation, 4);
  }
}

//##################################################################################################
void PostDoFMergeShader::setFocusTexture(const GLuint focusTextureID)
{
  if(d->focusTextureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, focusTextureID);
    glUniform1i(d->focusTextureLocation, 5);
  }
}

//##################################################################################################
void PostDoFMergeShader::setDownsampledFocusTexture(const GLuint downsampledFocusTextureID)
{
  if(d->downsampledFocusTextureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, downsampledFocusTextureID);
    glUniform1i(d->downsampledFocusTextureLocation, 6);
  }
}

//##################################################################################################
const std::string& PostDoFMergeShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/MergeDofShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

//##################################################################################################
void PostDoFMergeShader::getLocations(GLuint program, ShaderType shaderType)
{
  PostDoFBaseShader::getLocations(program, shaderType);
  d->downsampledTextureLocation      = glGetUniformLocation(program, "downsampledTextureSampler");
  d->focusTextureLocation            = glGetUniformLocation(program, "focusTextureSampler");
  d->downsampledFocusTextureLocation = glGetUniformLocation(program, "downsampledFocusTextureSampler");
}

}
