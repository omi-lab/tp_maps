#include "tp_maps/shaders/CalculateFocusShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/CalculateFocusShader.frag"}; return s;}
}


namespace detail
{
//##################################################################################################
struct CalculateFocusShaderPrivate::Private
{
  DepthOfFieldShaderParameters parameters;
  std::string fragSrc;

  //################################################################################################
  Private(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderExtendedFBO);

    std::string DOF_FRAG_VARS;

    DOF_FRAG_VARS += "const float depthOfField = " + std::to_string(parameters.depthOfField) + ";\n";
    DOF_FRAG_VARS += "const float fStop = " + std::to_string(parameters.fStop) + ";\n";

    // For depth calculation
    DOF_FRAG_VARS += "const float nearPlane = " + std::to_string(parameters.nearPlane) + ";\n";
    DOF_FRAG_VARS += "const float farPlane = " + std::to_string(parameters.farPlane) + ";\n";
    DOF_FRAG_VARS += "const float focalDistance = " + std::to_string(parameters.focalDistance) + ";\n";
    DOF_FRAG_VARS += "const float blurinessCutoffConstant = " + std::to_string(parameters.blurinessCutoffConstant) + ";\n";

    tp_utils::replace(fragSrc, "/*DOF_FRAG_VARS*/", DOF_FRAG_VARS);
  }

  GLint textureLocation {0};
  GLint depthLocation   {0};
  GLint normalsLocation {0};
  GLint specularLocation{0};

  GLint projectionMatrixLocation   {0};
  GLint invProjectionMatrixLocation{0};

  GLint pixelSizeLocation{0};

  //################################################################################################
  std::function<void(GLuint)> bindLocations(const std::function<void(GLuint)>& bindLocations = std::function<void(GLuint)>())
  {
    return bindLocations;
  }

  //################################################################################################
  std::function<void(GLuint)> getLocations(const std::function<void(GLuint)>& getLocations = std::function<void(GLuint)>())
  {
    return getLocations;
  }

};

//##################################################################################################
CalculateFocusShaderPrivate::CalculateFocusShaderPrivate(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  d(new Private(map, openGLProfile, parameters))
{

}
}

//##################################################################################################
CalculateFocusShader::CalculateFocusShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const DepthOfFieldShaderParameters& parameters):
  CalculateFocusShaderPrivate(map, openGLProfile, parameters),
  PostShader(map, openGLProfile, nullptr, d->fragSrc.data(), d->bindLocations(), d->getLocations() )
{

}



//##################################################################################################
CalculateFocusShader::~CalculateFocusShader()
{
  delete d;
}

void CalculateFocusShader::compile(const char* vertexShader,
                                     const char* fragmentShader,
                                     const std::function<void(GLuint)>& bindLocations,
                                     const std::function<void(GLuint)>& getLocations,
                                     ShaderType shaderType)
{
  FullScreenShader::compile(vertexShader, fragmentShader, d->bindLocations(bindLocations), d->getLocations(getLocations), shaderType);
}

}
