#ifndef tp_maps_GridLayer_h
#define tp_maps_GridLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class FontRenderer;
class SpriteTexture;

//##################################################################################################
enum class GridMode
{
  Fixed,
  Predefined,
  User
};

//##################################################################################################
enum class GridAxis
{
  XPlane,
  YPlane,
  ZPlane,
  Screen,
  Frame
};

//##################################################################################################
enum class GridHandles
{
  None,
  OutsideFrame
};

//##################################################################################################
struct GridColors
{
  glm::vec4 centralLines     {1.0f, 1.0f, 1.0f, 0.80f};
  glm::vec4 primaryLines     {1.0f, 1.0f, 1.0f, 0.30f};
  glm::vec4 intermediateLines{1.0f, 1.0f, 1.0f, 0.05f};
  glm::vec4 userLines        {1.0f, 1.0f, 0.8f, 1.00f};
};

//##################################################################################################
struct GridPredefinedLines
{
  std::vector<float> horizontalLines;
  std::vector<float> verticalLines;
};

//##################################################################################################
class TP_MAPS_EXPORT GridLayer: public Layer
{
  TP_DQ;
public:
  //################################################################################################
  GridLayer(const std::function<tp_maps::SpriteTexture*(tp_maps::Map*)>& makeTexture={});

  //################################################################################################
  ~GridLayer() override;

  //################################################################################################
  void setMode(GridMode mode);

  //################################################################################################
  GridMode mode() const;

  //################################################################################################
  void setAxis(GridAxis axis);

  //################################################################################################
  GridAxis axis() const;

  //################################################################################################
  void setHandles(GridHandles handles);

  //################################################################################################
  GridHandles handles() const;

  //################################################################################################
  void setPredefinedLines(const GridPredefinedLines& predefinedLines);

  //################################################################################################
  const GridPredefinedLines& predefinedLines() const;

  //################################################################################################
  //! Set a multiplier for the spacing between each graduation in the grid.
  /*!
  \param spacing Smaller is spacing, closer are the graduations to one another.
  */
  void setSpacing(float spacing);

  //################################################################################################
  float spacing() const;

  //################################################################################################
  //! Set an offset to move the grid vertically.
  /*!
  \param heightOffset Vertical offset to elevate the grid above ground level.
  */
  void setHeightOffset(float heightOffset);

  //################################################################################################
  float heightOffset() const;

  //################################################################################################
  //! Set a 2D offset to move the grid centre on the horizontal plane.
  /*!
  \param horizontalTranslationOffset Offset in xOy plane.
  */
  void setHorizontalTranslationOffset(const glm::vec2& horizontalTranslationOffset);

  //################################################################################################
  //! Set a forward direction to orientate the grid in the horizontal plane.
  /*!
  \param horizontalOrientation Forward vector to orientate the grid on the horizontal plane.
  */
  void setHorizontalOrientation(const glm::vec2& horizontalOrientation);

  //################################################################################################
  void setDefaultRenderPass(const RenderPass& defaultRenderPass) override;

protected:

  //################################################################################################
  void addedToMap() override;

  //################################################################################################
  void render(RenderInfo& renderInfo) override;
};

}

#endif
