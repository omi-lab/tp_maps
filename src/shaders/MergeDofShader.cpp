#include "tp_maps/shaders/MergeDofShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/MergeDofShader.frag"}; return s;}
}


namespace detail
{
//##################################################################################################
struct MergeDofShaderPrivate::Private
{
  DepthOfFieldShaderParameters parameters;
  std::string fragSrc;

  GLint downsampledTextureLocation{0};
  GLint focusTextureLocation{0};
  GLint downsampledFocusTextureLocation{0};

  //################################################################################################
  Private(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderExtendedFBO);
  }

  //################################################################################################
  std::function<void(GLuint)> bindLocations(const std::function<void(GLuint)>& bindLocations = std::function<void(GLuint)>())
  {
    return bindLocations;
  }

  //################################################################################################
  std::function<void(GLuint)> getLocations(const std::function<void(GLuint)>& getLocations = std::function<void(GLuint)>())
  {
    return [=](GLuint program)
    {
      downsampledTextureLocation          = glGetUniformLocation(program, "downsampledTextureSampler");
      focusTextureLocation                = glGetUniformLocation(program, "focusTextureSampler");
      downsampledFocusTextureLocation     = glGetUniformLocation(program, "downsampledFocusTextureSampler");

      if(getLocations)
        getLocations(program);
    };
  }

};

//##################################################################################################
MergeDofShaderPrivate::MergeDofShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  d(new Private(map, openGLProfile, parameters))
{

}
}

//##################################################################################################
MergeDofShader::MergeDofShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  MergeDofShaderPrivate(map, openGLProfile, parameters),
  PostShader(map, openGLProfile, nullptr, d->fragSrc.data(), d->bindLocations(), d->getLocations() )
{

}



//##################################################################################################
MergeDofShader::~MergeDofShader()
{
  delete d;
}

void MergeDofShader::compile(const char* vertexShader,
                                     const char* fragmentShader,
                                     const std::function<void(GLuint)>& bindLocations,
                                     const std::function<void(GLuint)>& getLocations,
                                     ShaderType shaderType)
{
  FullScreenShader::compile(vertexShader, fragmentShader, d->bindLocations(bindLocations), d->getLocations(getLocations), shaderType);
}

void MergeDofShader::setDownsampledTexture( const GLuint downsampledTextureID )
{
    if(d->downsampledTextureLocation>=0)
    {
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, downsampledTextureID);
      glUniform1i(d->downsampledTextureLocation, 4);
    }
}

void MergeDofShader::setFocusTexture( const GLuint focusTextureID )
{
    if(d->focusTextureLocation>=0)
    {
      glActiveTexture(GL_TEXTURE5);
      glBindTexture(GL_TEXTURE_2D, focusTextureID);
      glUniform1i(d->focusTextureLocation, 5);
    }
}

void MergeDofShader::setDownsampledFocusTexture( const GLuint downsampledFocusTextureID )
{
    if(d->downsampledFocusTextureLocation>=0)
    {
      glActiveTexture(GL_TEXTURE6);
      glBindTexture(GL_TEXTURE_2D, downsampledFocusTextureID);
      glUniform1i(d->downsampledFocusTextureLocation, 6);
    }
}

}
