#ifndef tp_maps_SpriteTexture_h
#define tp_maps_SpriteTexture_h

#include "tp_maps/Globals.h"

#include "tp_utils/RefCount.h"

#include <vector>
#include <array>
#include <functional>

namespace tp_maps
{
class Texture;

//##################################################################################################
struct TP_MAPS_EXPORT SpriteCoords
{
  std::array<glm::vec2, 4> coords;

  //################################################################################################
  static std::vector<SpriteCoords> oneOne(float w=1.0f, float h=1.0f)
  {
    SpriteCoords coords;
    coords.coords[0] = {0.0f,0.0f};
    coords.coords[1] = {   w,0.0f};
    coords.coords[2] = {   w,   h};
    coords.coords[3] = {0.0f,   h};
    return {coords};
  }

  //################################################################################################
  static std::vector<SpriteCoords> grid(size_t nx, size_t ny)
  {
    std::vector<SpriteCoords> sprites;
    sprites.resize(nx*ny);

    for(size_t y=0; y<ny; y++)
    {
      float fy0 = float(y  )/float(ny);
      float fy1 = float(y+1)/float(ny);

      for(size_t x=0; x<nx; x++)
      {
        float fx0 = float(x  )/float(nx);
        float fx1 = float(x+1)/float(nx);

        SpriteCoords& coords = sprites.at((y*nx)+x);
        coords.coords[0] = {fx0,fy0};
        coords.coords[1] = {fx1,fy0};
        coords.coords[2] = {fx1,fy1};
        coords.coords[3] = {fx0,fy1};
      }
    }

    return sprites;
  }
};

//##################################################################################################
class SpriteTexture
{
  TP_NONCOPYABLE(SpriteTexture);
  TP_REF_COUNT_OBJECTS("SpriteTexture");
public:
  //################################################################################################
  SpriteTexture()=default;

  //################################################################################################
  ~SpriteTexture();

  //################################################################################################
  //! Returns the texture that belongs to this sprite set
  Texture* texture() const;

  //################################################################################################
  const std::vector<SpriteCoords>& coords() const;

  //################################################################################################
  void setCoordsChangedCallback(std::function<void()> callback);

  //################################################################################################
  //! This should be called once before use
  /*!
  \param texture The texture object, SpriteTexture will take ownership of texture.
  */
  void setTexture(Texture* texture);

  //################################################################################################
  //! Call this each time the coords change
  void setCoords(const std::vector<SpriteCoords>& coords);

private:
  Texture* m_texture{nullptr};
  std::vector<SpriteCoords> m_coords;
  std::function<void()> m_callback;
};

}

#endif
