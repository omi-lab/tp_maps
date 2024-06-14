#ifndef tp_maps_CircleSectorLayer_h
#define tp_maps_CircleSectorLayer_h

#include "tp_maps/Layer.h"

#include "json.hpp"

namespace tp_maps
{

//##################################################################################################
struct CircleSectorParameters
{
  glm::vec4 fillColor     {1.0f, 0.0f, 0.0f, 1.0f};
  glm::vec4 borderColor   {1.0f, 0.0f, 0.0f, 1.0f};
  glm::vec4 startLineColor{1.0f, 0.0f, 0.0f, 1.0f};
  glm::vec4 endLineColor  {1.0f, 0.0f, 0.0f, 1.0f};

  //################################################################################################
  void saveState(nlohmann::json& j) const;

  //################################################################################################
  void loadState(const nlohmann::json& j);
};

//##################################################################################################
class TP_MAPS_EXPORT CircleSectorLayer: public Layer
{
  TP_DQ;
public:
  //################################################################################################
  CircleSectorLayer();

  //################################################################################################
  ~CircleSectorLayer() override;

  //################################################################################################
  void setParameters(const CircleSectorParameters& params);

  //################################################################################################
  const CircleSectorParameters& parameters() const;

  //################################################################################################
  void setAngleDegrees(float angleDegrees);

  //################################################################################################
  float angleDegrees() const;

  //################################################################################################
  void setDefaultRenderPass(const RenderPass& defaultRenderPass) override;

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;
};

}

#endif
