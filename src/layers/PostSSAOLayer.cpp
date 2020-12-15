#include "tp_maps/layers/PostSSAOLayer.h"
#include "tp_maps/shaders/PostSSAOShader.h"
#include "tp_maps/Map.h"

namespace tp_maps
{

//##################################################################################################
PostSSAOLayer::PostSSAOLayer(Map* map, RenderPass customRenderPass):
  PostLayer(map, customRenderPass)
{
  std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
  std::default_random_engine generator;
  std::vector<glm::vec3> ssaoKernel;
  for (unsigned int i = 0; i < 64; ++i)
  {
    glm::vec3 sample(
          randomFloats(generator) * 2.0f - 1.0f,
          randomFloats(generator) * 2.0f - 1.0f,
          randomFloats(generator)
          );
    sample  = glm::normalize(sample);
    sample *= randomFloats(generator);
    ssaoKernel.push_back(sample);
  }

}

//##################################################################################################
PostShader* PostSSAOLayer::makeShader()
{
  return map()->getShader<PostSSAOShader>();
}

}
