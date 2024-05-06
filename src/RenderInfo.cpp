#include "tp_maps/RenderInfo.h"

namespace tp_maps
{

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

  uint32_t r = (id & 0x000000FF) >>  0;
  uint32_t g = (id & 0x0000FF00) >>  8;
  uint32_t b = (id & 0x00FF0000) >> 16;

  return glm::vec4 (float(r) / 255.0f, float(g) / 255.0f, float(b) / 255.0f, 1.0f);
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
