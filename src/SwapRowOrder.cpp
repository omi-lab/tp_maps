#include "tp_maps/SwapRowOrder.h"

#include "tp_image_utils/ColorMap.h"
#include "tp_image_utils/ColorMapF.h"

#include <string.h>

namespace tp_maps
{

//##################################################################################################
template<typename T>
void swapY(size_t width, size_t height, T* pixels)
{
  std::vector<T> line{size_t(width)};
  T* c = line.data();
  size_t rowLengthBytes = size_t(width)*sizeof(T);
  size_t yMax = size_t(height)/2;
  for(size_t y=0; y<yMax; y++)
  {
    T* a{pixels + y*size_t(width)};
    T* b{pixels + (size_t(height-1)-y)*size_t(width)};

    memcpy(c, a, rowLengthBytes);
    memcpy(a, b, rowLengthBytes);
    memcpy(b, c, rowLengthBytes);
  }
}

//##################################################################################################
void swapRowOrder(size_t width, size_t height, tp_image_utils::ColorMap& image)
{
  swapRowOrder(width, height, image.data());
}

//##################################################################################################
void swapRowOrder(size_t width, size_t height, TPPixel* pixels)
{
  swapY(width, height, pixels);
}

//##################################################################################################
void swapRowOrder(size_t width, size_t height, tp_image_utils::ColorMapF& image)
{
  swapRowOrder(width, height, image.data());
}

//##################################################################################################
void TP_MAPS_EXPORT swapRowOrder(size_t width,
                                        size_t height,
                                        glm::vec4* pixels)
{
  swapY(width, height, pixels);
}

}
