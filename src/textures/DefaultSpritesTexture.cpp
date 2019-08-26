#include "tp_maps/textures/DefaultSpritesTexture.h"

namespace tp_maps
{

//##################################################################################################
DefaultSpritesTexture::DefaultSpritesTexture(Map* map):
  BasicTexture(map)
{
  TextureData texture;
  auto newData = new TPPixel[32*32];
  texture.data = newData;
  texture.w = 32;
  texture.h = 32;

  for(int y=0; y<32; y++)
  {
    float fy = float(std::abs(y-16));
    fy = fy*fy;
    for(int x=0; x<32; x++)
    {
      float fx = float(std::abs(x-16));
      float f = std::sqrt(fx*fx + fy);

      TPPixel& p = newData[y*32+x];
      p.a = (f>16)?0:255;
      p.r = uint8_t(255.0f-f);
      p.g = uint8_t(255.0f-f);
      p.b = uint8_t(255.0f-f);
    }
  }

  setImage(texture);
  texture.destroy();
}

}
