#ifndef tp_maps_Geometry3DLayer_h
#define tp_maps_Geometry3DLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
class Texture;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Geometry3DLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("Geometry3DLayer");
public:
  //################################################################################################
  Geometry3DLayer();

  //################################################################################################
  ~Geometry3DLayer()override;

  //################################################################################################
  /*!
  When you call setGeometry you also set the material to use for each mesh, in the material you can
  specify the textures by name to use for that mesh. The names in the material corrospond to the
  keys in this map.

  \param textures The map of name to texture, this will take ownership.
  */
  void setTextures(const std::unordered_map<tp_utils::StringID, Texture*>& textures);

  //################################################################################################
  const std::vector<Geometry3D>& geometry() const;

  //################################################################################################
  void setGeometry(const std::vector<Geometry3D>& geometry);

  //################################################################################################
  //! Call this to set the material of all geometry.
  void setMaterial(const Material& material);

  //################################################################################################
  enum class ShaderSelection
  {
    Material, //!< Render the 3D geometry as a shaded material using the MaterialShader.
    Image     //!< Render the 3D geometry as flat unshaded images using the ImageShader.
  };

  //################################################################################################
  void setShaderSelection(ShaderSelection shaderSelection);

  //################################################################################################
  ShaderSelection shaderSelection() const;

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
