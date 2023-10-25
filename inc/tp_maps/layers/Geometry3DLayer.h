#ifndef tp_maps_Geometry3DLayer_h
#define tp_maps_Geometry3DLayer_h

#include "tp_maps/Layer.h"

#include "tp_math_utils/Geometry3D.h"

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
class TP_MAPS_EXPORT Geometry3DLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("Geometry3DLayer");
public:
  //################################################################################################
  Geometry3DLayer(Geometry3DPool* geometry3DPool=nullptr);

  //################################################################################################
  ~Geometry3DLayer() override;

  //################################################################################################
  void setName(const tp_utils::StringID& name);

  //################################################################################################
  const tp_utils::StringID& name() const;

  //################################################################################################
  TexturePool* texturePool() const;

  //################################################################################################
  void setTextures(const std::unordered_map<tp_utils::StringID, tp_image_utils::ColorMap>& textures);

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
    Material,   //!< Render the 3D geometry as a shaded material using the G3DMaterialShader.
    Image,      //!< Render the 3D geometry as flat unshaded images using the G3DImageShader.
    XYZ,        //!< Write the frag xyz coords in world coords to the output buffer using the G3DXYZShader.
    Depth,      //!< Write the depth buffer to the output buffer.
    StaticLight //!< Render the 3D geometry as a shaded material ignoring lights and shadows.
  };

  //################################################################################################
  void setShaderSelection(ShaderSelection shaderSelection);

  //################################################################################################
  ShaderSelection shaderSelection() const;

  //################################################################################################
  void setAlternativeMaterials(const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials);

  //################################################################################################
  const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials() const;

  //################################################################################################
  //! Aditional UV transformations that will be multiplied with the material UV transformation.
  /*!
  If available these will be mapped 1 per a geometry.
  */
  void setUVTransformations(const std::vector<tp_math_utils::UVTransformation>& uvTransformations);

  //################################################################################################
  const std::vector<tp_math_utils::UVTransformation>& uvTransformations() const;


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
