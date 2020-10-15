#ifndef tp_maps_PointsLayer_h
#define tp_maps_PointsLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/SpriteTexture.h"
#include "tp_maps/shaders/PointSpriteShader.h"

#include "tp_utils/RefCount.h"

#include "glm/glm.hpp"

#include <functional>

namespace tp_maps
{
class Texture;


//##################################################################################################
class TP_MAPS_SHARED_EXPORT PointsLayer: public Layer
{
  TP_REF_COUNT_OBJECTS("PointsLayer");
public:
  //################################################################################################
  /*!
  \param spriteTexture The sprite texture, PointsLayer will take ownership of spriteTexture.
  */
  PointsLayer(SpriteTexture* spriteTexture);

  //################################################################################################
  ~PointsLayer() override;

  //################################################################################################
  void clearPoints();

  //################################################################################################
  void setPoints(const std::vector<PointSpriteShader::PointSprite>& points);

  //################################################################################################
  const std::vector<PointSpriteShader::PointSprite>& points() const;

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
