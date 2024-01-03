#include "tp_maps/shaders/PostAOMergeShader.h"

namespace tp_maps
{

//##################################################################################################
struct PostAOMergeShader::Private
{
  GLint ssaoTextureLocation{0};
};

//##################################################################################################
PostAOMergeShader::PostAOMergeShader(Map* map,
                                     tp_maps::ShaderProfile shaderProfile,
                                     const PostAOParameters& parameters):
  PostAOBaseShader(map, shaderProfile, parameters),
d(new Private())
{

}

//##################################################################################################
PostAOMergeShader::~PostAOMergeShader()
{
  delete d;
}

//##################################################################################################
void PostAOMergeShader::setSSAOTexture(const GLuint ssaoTextureID)
{
  if(d->ssaoTextureLocation>=0)
  {
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, ssaoTextureID);
    glUniform1i(d->ssaoTextureLocation, 4);
  }
}

//##################################################################################################
const char* PostAOMergeShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/MergeAmbientOcclusionShader.frag"};
  fragSrcScratch = s.dataStr(shaderProfile(), shaderType);

  const auto& parameters = this->parameters();

  std::string AO_FRAG_VARS;

  AO_FRAG_VARS += "const bool showAOTexture=" + std::string( (parameters.showTexture ? "true" : "false") ) + ";\n";
  AO_FRAG_VARS += "const float radius=" + std::to_string(parameters.radius) + ";\n";
  AO_FRAG_VARS += "#define N_SAMPLES " + std::to_string(parameters.nSamples) + "\n";
  AO_FRAG_VARS += "const float bias=" + std::to_string(parameters.bias) + ";\n";
  AO_FRAG_VARS += "const float boostUpperThreshold="  + std::to_string(parameters.boostUpperThreshold) + ";\n";
  AO_FRAG_VARS += "const float boostLowerThreshold="  + std::to_string(parameters.boostLowerThreshold) + ";\n";
  AO_FRAG_VARS += "const float boostUpperFactor="     + std::to_string(parameters.boostUpperFactor) + ";\n";
  AO_FRAG_VARS += "const float boostLowerFactor="     + std::to_string(parameters.boostLowerFactor) + ";\n";

  tp_utils::replace(fragSrcScratch, "/*AO_FRAG_VARS*/", AO_FRAG_VARS);

  return fragSrcScratch.c_str();
}

//##################################################################################################
void PostAOMergeShader::getLocations(GLuint program, ShaderType shaderType)
{
  PostAOBaseShader::getLocations(program, shaderType);
  d->ssaoTextureLocation = glGetUniformLocation(program, "ssaoTextureSampler");
}

}
