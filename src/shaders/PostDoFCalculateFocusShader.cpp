#include "tp_maps/shaders/PostDoFCalculateFocusShader.h"

namespace tp_maps
{
//##################################################################################################
const char* PostDoFCalculateFocusShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/CalculateFocusShader.frag"};
  fragSrcScratch = s.dataStr(shaderProfile(), shaderType);

  const auto& parameters = this->parameters();

  std::string DOF_FRAG_VARS;

  DOF_FRAG_VARS += "const float depthOfField = " + std::to_string(parameters.depthOfField) + ";\n";
  DOF_FRAG_VARS += "const float fStop = " + std::to_string(parameters.fStop) + ";\n";

  // For depth calculation
  DOF_FRAG_VARS += "const float nearPlane = " + std::to_string(parameters.nearPlane) + ";\n";
  DOF_FRAG_VARS += "const float farPlane = " + std::to_string(parameters.farPlane) + ";\n";
  DOF_FRAG_VARS += "const float focalDistance = " + std::to_string(parameters.focalDistance) + ";\n";
  DOF_FRAG_VARS += "const float blurinessCutoffConstant = " + std::to_string(parameters.blurinessCutoffConstant) + ";\n";

  tp_utils::replace(fragSrcScratch, "/*DOF_FRAG_VARS*/", DOF_FRAG_VARS);

  return fragSrcScratch.c_str();
}
}
