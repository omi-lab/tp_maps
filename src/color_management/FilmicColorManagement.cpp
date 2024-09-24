#include "tp_maps/color_management/FilmicColorManagement.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/Resources.h"

namespace tp_maps
{

namespace
{
constexpr float gamma{2.2f};

// Piecewise transformation to filmic (compress saturation on the largest part of the range)
// /!\ Port those changes to glsl if altering anything
constexpr float pieceWiseThreshold {0.95f};
constexpr float lowerPieceSlope {0.9f}; // Scale saturation by a factor of 0.9
constexpr float upperPieceSlope = (1.f - lowerPieceSlope*pieceWiseThreshold) / (1.f-pieceWiseThreshold);
constexpr float upperPieceAffineOffset = (lowerPieceSlope - upperPieceSlope) * pieceWiseThreshold;

// Reverse transformation
constexpr float revPieceWiseThreshold = lowerPieceSlope * pieceWiseThreshold;
constexpr float revLowerPieceSlope  = 1.f/lowerPieceSlope;
constexpr float revUpperPieceSlope = (1.f - revLowerPieceSlope*revPieceWiseThreshold) / (1.f-revPieceWiseThreshold);
constexpr float revUpperPieceAffineOffset = (revLowerPieceSlope - revUpperPieceSlope) * revPieceWiseThreshold;
}

//##################################################################################################
FilmicColorManagement::~FilmicColorManagement()=default;

//##################################################################################################
glm::vec4 FilmicColorManagement::toLinear(const glm::vec4& color) const
{
  return glm::vec4(glm::pow(glm::vec3(color), glm::vec3(gamma)), color.w);
}

//##################################################################################################
glm::vec3 FilmicColorManagement::toLinear(const glm::vec3& color) const
{
  glm::vec3 hsv = tp_math_utils::rgb2hsv(glm::pow(color, glm::vec3(gamma)));
  float isUpperPiece = (hsv.y > revPieceWiseThreshold);
  hsv.y = (1.f - isUpperPiece)*(revLowerPieceSlope * hsv.y) + isUpperPiece*(revUpperPieceAffineOffset + revUpperPieceSlope * hsv.y);
  hsv.y = std::clamp(hsv.y, 0.f, 1.f);
  return tp_math_utils::hsv2rgb(hsv);
}

//##################################################################################################
glm::vec4 FilmicColorManagement::fromLinear(const glm::vec4& color) const
{
  return glm::vec4(glm::pow(glm::vec3(color), glm::vec3(1.0f/gamma)), color.w);
}

//##################################################################################################
glm::vec3 FilmicColorManagement::fromLinear(const glm::vec3& color) const
{
  glm::vec3 hsv = tp_math_utils::rgb2hsv(color);
  float isUpperPiece = (hsv.y > pieceWiseThreshold);
  hsv.y = (1.f - isUpperPiece)*(lowerPieceSlope * hsv.y) + isUpperPiece*(upperPieceAffineOffset + upperPieceSlope * hsv.y);
  hsv.y = std::clamp(hsv.y, 0.f, 1.f);
  return glm::pow(tp_math_utils::hsv2rgb(hsv), glm::vec3(1.0f/gamma));
}

//##################################################################################################
std::string FilmicColorManagement::glsl() const
{
  return tp_utils::resource("/tp_maps/FilmicColorManagement.glsl").data;
}

}
