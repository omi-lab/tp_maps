#include "tp_maps/shaders/DownsampleShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/DownsampleShader.frag"}; return s;}
}


namespace detail
{
//##################################################################################################
struct DownsampleShaderPrivate::Private
{
  DepthOfFieldShaderParameters parameters;
  std::string fragSrc;

  //################################################################################################
  Private(tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderExtendedFBO);
  }

  //################################################################################################
  std::function<void(GLuint)> bindLocations(const std::function<void(GLuint)>& bindLocations)
  {
    return bindLocations;
  }

  //################################################################################################
  std::function<void(GLuint)> getLocations(const std::function<void(GLuint)>& getLocations)
  {
    return [=](GLuint program)
    {
      if(getLocations)
        getLocations(program);
    };
  }

};

//##################################################################################################
DownsampleShaderPrivate::DownsampleShaderPrivate(tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  d(new Private(openGLProfile, parameters))
{

}
}

//##################################################################################################
DownsampleShader::DownsampleShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  DownsampleShaderPrivate(openGLProfile, parameters),
  PostShader(map, openGLProfile, nullptr, d->fragSrc.data() )
{

}



//##################################################################################################
DownsampleShader::~DownsampleShader()
{
  delete d;
}

void DownsampleShader::compile(const char* vertexShader,
                                     const char* fragmentShader,
                                     const std::function<void(GLuint)>& bindLocations,
                                     const std::function<void(GLuint)>& getLocations,
                                     ShaderType shaderType)
{
  FullScreenShader::compile(vertexShader, fragmentShader, d->bindLocations(bindLocations), d->getLocations(getLocations), shaderType);
}

}
