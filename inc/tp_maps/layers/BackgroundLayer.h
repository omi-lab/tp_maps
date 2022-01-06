#ifndef tp_maps_BackgroundLayer_h
#define tp_maps_BackgroundLayer_h

#include "tp_maps/Layer.h"

#include "tp_utils/RefCount.h"

namespace tp_maps
{
class TexturePool;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT BackgroundLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("BackgroundLayer");
public:
  //################################################################################################
  BackgroundLayer(TexturePool* texturePool);

  //################################################################################################
  ~BackgroundLayer() override;

  //################################################################################################
  enum class Mode
  {
    Spherical,
    TransparentPattern
  };

  //################################################################################################
  Mode mode() const;

  //################################################################################################
  void setMode(Mode mode);



  //################################################################################################
  const tp_utils::StringID& textureName() const;

  //################################################################################################
  void setTextureName(const tp_utils::StringID& textureName);

  //################################################################################################
  float rotationFactor() const;

  //################################################################################################
  //! A rotation value between 0 and 1.
  void setRotationFactor(float rotationFactor);



  //################################################################################################
  float gridSpacing() const;

  //################################################################################################
  void setGridSpacing(float gridSpacing);

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
