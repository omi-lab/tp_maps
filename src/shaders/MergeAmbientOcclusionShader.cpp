#include "tp_maps/shaders/MergeAmbientOcclusionShader.h"

#include "tp_utils/DebugUtils.h"

namespace tp_maps
{

namespace
{
ShaderResource& fragShaderStr(){static ShaderResource s{"/tp_maps/MergeAmbientOcclusionShader.frag"}; return s;}
}

namespace detail
{
//##################################################################################################
struct MergeAmbientOcclusionShaderPrivate::Private
{
  AmbientOcclusionParameters parameters;

  std::string fragSrc;

  GLint ssaoTextureLocation{0};

  //################################################################################################
  Private(tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters_):
    parameters(parameters_)
  {
    fragSrc = fragShaderStr().dataStr(openGLProfile, ShaderType::RenderExtendedFBO);

    std::string AO_FRAG_VARS;

    AO_FRAG_VARS += "const bool showAOTexture=" + std::string( (parameters.showTexture ? "true" : "false") ) + ";\n";
    AO_FRAG_VARS += "const float radius=" + std::to_string(parameters.radius) + ";\n";
    AO_FRAG_VARS += "#define N_SAMPLES " + std::to_string(parameters.nSamples) + "\n";
    AO_FRAG_VARS += "const float bias=" + std::to_string(parameters.bias) + ";\n";
    AO_FRAG_VARS += "const float boostUpperThreshold="  + std::to_string(parameters.boostUpperThreshold) + ";\n";
    AO_FRAG_VARS += "const float boostLowerThreshold="  + std::to_string(parameters.boostLowerThreshold) + ";\n";
    AO_FRAG_VARS += "const float boostUpperFactor="     + std::to_string(parameters.boostUpperFactor) + ";\n";
    AO_FRAG_VARS += "const float boostLowerFactor="     + std::to_string(parameters.boostLowerFactor) + ";\n";

    tp_utils::replace(fragSrc, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);
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
      ssaoTextureLocation          = glGetUniformLocation(program, "ssaoTextureSampler");

      if(getLocations)
        getLocations(program);
    };
  }
};

//##################################################################################################
MergeAmbientOcclusionShaderPrivate::MergeAmbientOcclusionShaderPrivate(tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters):
  d(new Private(openGLProfile, parameters))
{

}

}

//##################################################################################################
MergeAmbientOcclusionShader::MergeAmbientOcclusionShader(Map* map, tp_maps::OpenGLProfile openGLProfile, const AmbientOcclusionParameters& parameters_):
  MergeAmbientOcclusionShaderPrivate(openGLProfile, parameters_),
  PostShader(map, openGLProfile, nullptr, d->fragSrc.data(), d->bindLocations(), d->getLocations() )
{
  tpWarning() << "merge ambient occlusion shader constructor";
}

//##################################################################################################
void MergeAmbientOcclusionShader::setSSAOTexture(const GLuint ssaoTextureID)
{
    if(d->ssaoTextureLocation>=0)
    {
      glActiveTexture(GL_TEXTURE4);
      glBindTexture(GL_TEXTURE_2D, ssaoTextureID);
      glUniform1i(d->ssaoTextureLocation, 4);
    }
}

}
