#ifndef tp_maps_PostDoFBaseShader_h
#define tp_maps_PostDoFBaseShader_h

#include "tp_maps/shaders/PostShader.h"

namespace tp_maps
{

//##################################################################################################
struct PostDoFParameters
{
  bool enabled{false};
  float depthOfField{2.0f}; // focal distance
  float fStop{5.0f};

  // Calculate focus parameters
  float nearPlane{0.1f};
  float farPlane{50.0f};
  float focalDistance{8.0f};
  float blurinessCutoffConstant{10.0f};

  // fuzzy equality function used to indicate that shaders need to be recompiled
  bool fuzzyEquals(const PostDoFParameters& other) const;
};

//##################################################################################################
//! Part of the DoF shaders.
class TP_MAPS_EXPORT PostDoFBaseShader: public PostShader
{
  TP_DQ;
public:
  //################################################################################################
  PostDoFBaseShader(Map* map,
                    tp_maps::ShaderProfile shaderProfile,
                    const PostDoFParameters& parameters);

  //################################################################################################
  ~PostDoFBaseShader() override;

  //################################################################################################
  const PostDoFParameters& parameters() const;

  //################################################################################################
  void setProjectionNearAndFar(float near, float far);

  //################################################################################################
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;
};


}

#endif
