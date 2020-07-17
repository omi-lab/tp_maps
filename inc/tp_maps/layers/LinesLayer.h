#ifndef tp_maps_LinesLayer_h
#define tp_maps_LinesLayer_h

#include "tp_maps/layers/Geometry3DLayer.h"

#include "tp_math_utils/Geometry3D.h"

#include "glm/glm.hpp"

namespace tp_maps
{
class LinesLayer;
class Texture;

//##################################################################################################
struct TP_MAPS_SHARED_EXPORT Lines
{
  std::vector<glm::vec3> lines;
  glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};
  GLenum mode{GL_LINE_LOOP};
};

//##################################################################################################
class TP_MAPS_SHARED_EXPORT LinesLayer: public Layer
{
public:
  //################################################################################################
  LinesLayer();

  //################################################################################################
  ~LinesLayer()override;

  //################################################################################################
  const std::vector<Lines>& lines()const;

  //################################################################################################
  void setLines(const std::vector<Lines>& lines);

  //################################################################################################
  void setLinesFromGeometryNormals(const std::vector<Geometry3D>& geometry, float scale);

  //################################################################################################
  const glm::mat4& objectMatrix()const;

  //################################################################################################
  void setObjectMatrix(const glm::mat4& objectMatrix);

  //################################################################################################
  float lineWidth()const;

  //################################################################################################
  void setLineWidth(float lineWidth);

protected:
  //################################################################################################
  virtual glm::mat4 calculateMatrix() const;

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
