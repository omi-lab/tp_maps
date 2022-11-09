#ifndef tp_maps_SwapY_h
#define tp_maps_SwapY_h

#include "tp_maps/Globals.h"

#include "tp_utils/TPPixel.h"

#include <functional>

namespace tp_image_utils
{
class ColorMap;
class ColorMapF;
}

namespace tp_maps
{

//##################################################################################################
void TP_MAPS_SHARED_EXPORT swapRowOrder(size_t width,
                                        size_t height,
                                        tp_image_utils::ColorMap& image);

//##################################################################################################
void TP_MAPS_SHARED_EXPORT swapRowOrder(size_t width,
                                        size_t height,
                                        TPPixel* pixels);

//##################################################################################################
void TP_MAPS_SHARED_EXPORT swapRowOrder(size_t width,
                                        size_t height,
                                        tp_image_utils::ColorMapF& image);

//##################################################################################################
void TP_MAPS_SHARED_EXPORT swapRowOrder(size_t width,
                                        size_t height,
                                        glm::vec4* pixels);

}

#endif
