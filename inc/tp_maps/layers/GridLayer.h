#ifndef tp_maps_GridLayer_h
#define tp_maps_GridLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class FontRenderer;

//##################################################################################################
class TP_MAPS_SHARED_EXPORT GridLayer: public Layer
{
public:
  //################################################################################################
  GridLayer(float scale=1.0f);

  //################################################################################################
  ~GridLayer() override;

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

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
