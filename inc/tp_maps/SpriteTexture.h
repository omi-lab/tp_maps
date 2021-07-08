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
struct TP_MAPS_SHARED_EXPORT SpriteCoords
{
  std::array<glm::vec2, 4> coords;

  static std::vector<SpriteCoords> oneOne(float w=1.0f, float h=1.0f)
  {
    SpriteCoords coords;
    coords.coords[0] = {0.0f,0.0f};
    coords.coords[1] = {   w,0.0f};
    coords.coords[2] = {   w,   h};
    coords.coords[3] = {0.0f,   h};
    return {coords};
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
