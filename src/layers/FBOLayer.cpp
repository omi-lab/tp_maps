#include "tp_maps/layers/FBOLayer.h"
#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/shaders/DepthImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"

#include "tp_math_utils/Transformation.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct FBOLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FBOLayer::Private");
  TP_NONCOPYABLE(Private);

  glm::vec2 origin{0.75f, 0.75f};
  glm::vec2 size  {0.20f, 0.20f};

  FBOLayerSource source;
  size_t index;

  ImageShader::VertexBuffer* vertexBuffer{nullptr};
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(FBOLayerSource source_, size_t index_):
    source(source_),
    index(index_)
  {

  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    delete vertexBuffer;
    vertexBuffer=nullptr;
  }
};

//##################################################################################################
FBOLayer::FBOLayer(FBOLayerSource source, size_t index):
  d(new Private(source, index))
{
  setDefaultRenderPass(RenderPass::GUI);
}

//##################################################################################################
FBOLayer::~FBOLayer()
{
  d->deleteVertexBuffers();
  delete d;
}

//##################################################################################################
void FBOLayer::setImageCoords(const glm::vec2& origin, const glm::vec2& size)
{
  d->origin = origin;
  d->size = size;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void FBOLayer::setSource(FBOLayerSource source, size_t index)
{
  d->source = source;
  d->index = index;
  update();
}

//##################################################################################################
void FBOLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass())
    return;

  GLuint textureID{0};
  ImageShader* shader{nullptr};
  switch(d->source)
  {
  case FBOLayerSource::ReflectionColor:
  {
    textureID = map()->reflectionTexture();
    shader = static_cast<ImageShader*>(map()->getShader<ImageShader>());
    break;
  }

  case FBOLayerSource::ReflectionDepth:
  {
    textureID = map()->reflectionDepth();
    shader = static_cast<ImageShader*>(map()->getShader<DepthImageShader>());
    break;
  }

  case FBOLayerSource::LightColor:
  {
    if(d->index < map()->lightTextures().size())
      textureID = map()->lightTextures().at(d->index).textureID;
    shader = static_cast<ImageShader*>(map()->getShader<ImageShader>());
    break;
  }

  case FBOLayerSource::LightDepth:
  {
    if(d->index < map()->lightTextures().size())
      textureID = map()->lightTextures().at(d->index).depthID;
    shader = static_cast<ImageShader*>(map()->getShader<DepthImageShader>());
    break;
  }
  };

  if(!shader || shader->error())
    return;

  if(!textureID)
    return;

  if(d->updateVertexBuffer)
  {
    auto w = d->size.x;
    auto h = d->size.y;

    auto topRight    = glm::vec3(d->origin + glm::vec2(w, 0), 0.0f);
    auto bottomRight = glm::vec3(d->origin + glm::vec2(w, h), 0.0f);
    auto bottomLeft  = glm::vec3(d->origin + glm::vec2(0, h), 0.0f);
    auto topLeft     = glm::vec3(d->origin + glm::vec2(0, 0), 0.0f);

    std::vector<GLuint> indexes{3,2,1,0};
    std::vector<ImageShader::Vertex> verts;

    verts.clear();
    verts.push_back(ImageShader::Vertex(topRight   , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}));
    verts.push_back(ImageShader::Vertex(bottomRight, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}));
    verts.push_back(ImageShader::Vertex(bottomLeft , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}));
    verts.push_back(ImageShader::Vertex(topLeft    , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}));

    delete d->vertexBuffer;
    d->vertexBuffer = shader->generateVertexBuffer(map(), indexes, verts);
    d->updateVertexBuffer=false;
  }

  glm::mat4 matrix(1.0f);
  matrix = glm::translate(matrix, {-1.0f, 1.0f, 0.0f});
  matrix = glm::scale(matrix, {2.0f, -2.0f, 1.0f});

  shader->use();
  shader->setMatrix(matrix);
  shader->setTexture(textureID);

  shader->draw(GL_TRIANGLE_FAN, d->vertexBuffer, glm::vec4(1.0f));
}

//##################################################################################################
void FBOLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}