#ifndef tp_maps_GridLayer_h
#define tp_maps_GridLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class FontRenderer;

//##################################################################################################
enum class GridMode
{
  Fixed,
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
class TP_MAPS_EXPORT GridLayer: public Layer
{
  TP_DQ;
public:
  //################################################################################################
  GridLayer(float scale = 1.0f, const glm::vec3& gridColor = {0.05f, 0.05f, 0.9f});

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
  //! Set a multiplier for the spacing between each graduation in the grid.
  /*!
  \param spacing Smaller is spacing, closer are the graduations to one another.
  */
  void setSpacing(float spacing);

  //################################################################################################
  float spacing() const;

  //##################################################################################################
  //! Toggle between a 2D overlay grid and grid in perspective.
  /*!
  \param gridAs2DOverlay True to make the grid a 2D overlay, false to have it on the ground as perspective.
  */
  void setGridAs2DOverlay(bool gridAs2DOverlay);

  //##################################################################################################
  float gridAs2DOverlay() const;

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
  //! Set the font that will be used to labels
  /*!
  This sets the font that will be used to draw the grid labels.
  \note This does not take ownership.
  \param font The font to use for drawing grid labels.
  */
  void setFont(FontRenderer* font);

protected:
  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void invalidateBuffers() override;
};

}

#endif
