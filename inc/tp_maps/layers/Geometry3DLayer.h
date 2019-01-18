#ifndef tp_maps_Geometry3DLayer_h
#define tp_maps_Geometry3DLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/shaders/MaterialShader.h"

#include "tp_math_utils/Geometry3D.h"

namespace tp_maps
{
class Texture;

//##################################################################################################
struct Geometry3D
{
  tp_math_utils::Geometry3D geometry;
  MaterialShader::Material material;
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Geometry3DLayer: public Layer
{
public:
  //################################################################################################
  Geometry3DLayer(Texture* texture=nullptr);

  //################################################################################################
  ~Geometry3DLayer()override;

  //################################################################################################
  const std::vector<Geometry3D>& geometry()const;

  //################################################################################################
  void setGeometry(const std::vector<Geometry3D>& geometry);

  //################################################################################################
  const glm::mat4& objectMatrix()const;

  //################################################################################################
  void setObjectMatrix(const glm::mat4& objectMatrix);

  //################################################################################################
  //! Call this to set the lighting.
  void setLight(const MaterialShader::Light& light);

  //################################################################################################
  //! Call this to set the material of all geometry.
  void setMaterial(const MaterialShader::Material& material);

  //################################################################################################
  enum class ShaderType
  {
    Material, //!< Render the 3D geometry as a shaded material using the MaterialShader.
    Image     //!< Render the 3D geometry as flat unshaded images using the ImageShader.
  };

  //################################################################################################
  void setShaderType(ShaderType shaderType);

  //################################################################################################
  ShaderType shaderType() const;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
