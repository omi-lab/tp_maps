#ifndef tp_maps_Geometry3DLayer_h
#define tp_maps_Geometry3DLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_image_utils
{
class ColorMap;
}

namespace tp_maps
{
class TexturePool;
class Geometry3DPool;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT Geometry3DLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("Geometry3DLayer");
public:
  //################################################################################################
  Geometry3DLayer();

  //################################################################################################
  ~Geometry3DLayer() override;

  //################################################################################################
  void setName(const tp_utils::StringID& name);

  //################################################################################################
  const tp_utils::StringID& name() const;

  //################################################################################################
  void setTexturePool(TexturePool* texturePool);

  //################################################################################################
  TexturePool* texturePool() const;

  //################################################################################################
  void setTextures(const std::unordered_map<tp_utils::StringID, tp_image_utils::ColorMap>& textures);

  //################################################################################################
  void setGeometry3DPool(Geometry3DPool* geometry3DPool);

  //################################################################################################
  Geometry3DPool* geometry3DPool() const;

  //################################################################################################
  void setGeometry(const std::vector<tp_math_utils::Geometry3D>& geometry);

  //################################################################################################
  void viewGeometry(const std::function<void(const std::vector<tp_math_utils::Geometry3D>&)>& closure) const;

  //################################################################################################
  void viewGeometry(const std::function<void(const std::vector<tp_math_utils::Geometry3D>&, const std::vector<tp_math_utils::Material>&)>& closure) const;

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

  //################################################################################################
  void setAlternativeMaterials(const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials);

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials() const;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
