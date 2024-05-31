#ifndef tp_maps_RulerLayer_h
#define tp_maps_RulerLayer_h

#include "tp_maps/Layer.h"

namespace tp_maps
{
class FontRenderer;

//##################################################################################################
class TP_MAPS_EXPORT RulerLayer: public Layer
{
  TP_DQ;
public:
  //################################################################################################
  RulerLayer(float scale=1.0f);

  //################################################################################################
  ~RulerLayer() override;

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
