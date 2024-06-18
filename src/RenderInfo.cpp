#include "tp_maps/RenderInfo.h"

#ifdef TP_DISTINCT_PICKING_COLORS
#include "tp_utils/TPPixel.h"
#endif

namespace tp_maps
{
#ifdef TP_DISTINCT_PICKING_COLORS
namespace
{
//##################################################################################################
const TPPixel* indexColors()
{
  static const TPPixel indexcolors[] =
  {
    TPPixel("#000000"), TPPixel("#FFFF00"), TPPixel("#1CE6FF"), TPPixel("#FF34FF"), TPPixel("#FF4A46"), TPPixel("#008941"), TPPixel("#006FA6"), TPPixel("#A30059"),
    TPPixel("#FFDBE5"), TPPixel("#7A4900"), TPPixel("#0000A6"), TPPixel("#63FFAC"), TPPixel("#B79762"), TPPixel("#004D43"), TPPixel("#8FB0FF"), TPPixel("#997D87"),
    TPPixel("#5A0007"), TPPixel("#809693"), TPPixel("#FEFFE6"), TPPixel("#1B4400"), TPPixel("#4FC601"), TPPixel("#3B5DFF"), TPPixel("#4A3B53"), TPPixel("#FF2F80"),
    TPPixel("#61615A"), TPPixel("#BA0900"), TPPixel("#6B7900"), TPPixel("#00C2A0"), TPPixel("#FFAA92"), TPPixel("#FF90C9"), TPPixel("#B903AA"), TPPixel("#D16100"),
    TPPixel("#DDEFFF"), TPPixel("#000035"), TPPixel("#7B4F4B"), TPPixel("#A1C299"), TPPixel("#300018"), TPPixel("#0AA6D8"), TPPixel("#013349"), TPPixel("#00846F"),
    TPPixel("#372101"), TPPixel("#FFB500"), TPPixel("#C2FFED"), TPPixel("#A079BF"), TPPixel("#CC0744"), TPPixel("#C0B9B2"), TPPixel("#C2FF99"), TPPixel("#001E09"),
    TPPixel("#00489C"), TPPixel("#6F0062"), TPPixel("#0CBD66"), TPPixel("#EEC3FF"), TPPixel("#456D75"), TPPixel("#B77B68"), TPPixel("#7A87A1"), TPPixel("#788D66"),
    TPPixel("#885578"), TPPixel("#FAD09F"), TPPixel("#FF8A9A"), TPPixel("#D157A0"), TPPixel("#BEC459"), TPPixel("#456648"), TPPixel("#0086ED"), TPPixel("#886F4C"),

    TPPixel("#34362D"), TPPixel("#B4A8BD"), TPPixel("#00A6AA"), TPPixel("#452C2C"), TPPixel("#636375"), TPPixel("#A3C8C9"), TPPixel("#FF913F"), TPPixel("#938A81"),
    TPPixel("#575329"), TPPixel("#00FECF"), TPPixel("#B05B6F"), TPPixel("#8CD0FF"), TPPixel("#3B9700"), TPPixel("#04F757"), TPPixel("#C8A1A1"), TPPixel("#1E6E00"),
    TPPixel("#7900D7"), TPPixel("#A77500"), TPPixel("#6367A9"), TPPixel("#A05837"), TPPixel("#6B002C"), TPPixel("#772600"), TPPixel("#D790FF"), TPPixel("#9B9700"),
    TPPixel("#549E79"), TPPixel("#FFF69F"), TPPixel("#201625"), TPPixel("#72418F"), TPPixel("#BC23FF"), TPPixel("#99ADC0"), TPPixel("#3A2465"), TPPixel("#922329"),
    TPPixel("#5B4534"), TPPixel("#FDE8DC"), TPPixel("#404E55"), TPPixel("#0089A3"), TPPixel("#CB7E98"), TPPixel("#A4E804"), TPPixel("#324E72"), TPPixel("#6A3A4C"),
    TPPixel("#83AB58"), TPPixel("#001C1E"), TPPixel("#D1F7CE"), TPPixel("#004B28"), TPPixel("#C8D0F6"), TPPixel("#A3A489"), TPPixel("#806C66"), TPPixel("#222800"),
    TPPixel("#BF5650"), TPPixel("#E83000"), TPPixel("#66796D"), TPPixel("#DA007C"), TPPixel("#FF1A59"), TPPixel("#8ADBB4"), TPPixel("#1E0200"), TPPixel("#5B4E51"),
    TPPixel("#C895C5"), TPPixel("#320033"), TPPixel("#FF6832"), TPPixel("#66E1D3"), TPPixel("#CFCDAC"), TPPixel("#D0AC94"), TPPixel("#7ED379"), TPPixel("#012C58")
  };

  return indexcolors;
}
}
#endif

//##################################################################################################
uint32_t RenderInfo::pickingID(const PickingDetails& details)
{
  uint32_t id = nextID;
  nextID += details.count;
  pickingDetails.push_back(details);
  return id;
}

//##################################################################################################
glm::vec4 RenderInfo::pickingIDMat(const PickingDetails& details)
{
  uint32_t id = pickingID(details);

#ifdef TP_DISTINCT_PICKING_COLORS
  return indexColors()[id%256].toFloat4<glm::vec4>();
#else
  uint32_t r = (id & 0x000000FF) >>  0;
  uint32_t g = (id & 0x0000FF00) >>  8;
  uint32_t b = (id & 0x00FF0000) >> 16;

  return glm::vec4 (float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
#endif
}
//##################################################################################################
uint32_t RenderInfo::pickingIDFromColor(uint32_t r, uint32_t g, uint32_t b)
{
#ifdef TP_DISTINCT_PICKING_COLORS
  auto lookup = indexColors();
  for(size_t i=0; i<256; i++)
  {
    const auto& c = lookup[i];
    if(c.r == r && c.g==g && c.b==b)
      return uint32_t(i);
  }
  return 0;
#else
  uint32_t value = r;
  value |= (g<<8);
  value |= (b<<16);

  return value;
#endif
}

//##################################################################################################
void RenderInfo::resetPicking()
{
  pickingDetails.clear();
  pickingDetails.emplace_back();
  nextID = 1;
}

//##################################################################################################
ShaderType RenderInfo::shaderType() const
{
  if(pass == RenderPass::LightFBOs)
    return ShaderType::Light;

  if(pass == RenderPass::Picking || pass == RenderPass::PickingGUI3D)
    return ShaderType::Picking;

  if(extendedFBO == ExtendedFBO::Yes)
    return ShaderType::RenderExtendedFBO;

  return ShaderType::Render;
}

}
