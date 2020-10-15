#include "tp_maps/textures/DefaultSpritesTexture.h"

namespace tp_maps
{

//##################################################################################################
DefaultSpritesTexture::DefaultSpritesTexture(Map* map):
  BasicTexture(map)
{
  tp_image_utils::ColorMap texture(32, 32, nullptr);
  auto newData = texture.data();

  for(int y=0; y<32; y++)
  {
    float fy = float(std::abs(y-16));
    fy = fy*fy;
    for(int x=0; x<32; x++)
    {
      float fx = float(std::abs(x-16));
      float f = std::min(255.0f, std::sqrt(fx*fx + fy));
      float fc = std::min(255.0f, f*6.0f);

      TPPixel& p = newData[y*32+x];
      p.a = (f>=16)?0:255;
      p.r = uint8_t(255.0f-fc);
      p.g = uint8_t(255.0f-fc);
      p.b = uint8_t(255.0f-fc);
    }
  }

  setImage(texture);
}

}
