#include "tp_maps/textures/DefaultSpritesTexture.h"

namespace tp_maps
{

//##################################################################################################
DefaultSpritesTexture::DefaultSpritesTexture(Map* map):
  BasicTexture(map)
{
  TextureData texture;
  texture.data = new TPPixel[32*32];
  texture.w = 32;
  texture.h = 32;

  for(int y=0; y<32; y++)
  {
    float fy = abs(y-16);
    fy = fy*fy;
    for(int x=0; x<32; x++)
    {
      float fx = abs(x-16);
      float f = sqrt(fx*fx + fy);

      TPPixel& p = texture.data[y*32+x];
      p.a = (f>16)?0:255;
      p.r = 255-f;
      p.g = 255-f;
      p.b = 255-f;
    }
  }

  setImage(texture);
  texture.destroy();
}

}
