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

  FBOLayerSource source;
  size_t index;

  glm::vec2 origin{0.75f, 0.75f};
  glm::vec2 size  {0.20f, 0.20f};

  ImageShader::VertexBuffer* vertexBuffer{nullptr};
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(FBOLayerSource source_,
          size_t index_,
          const glm::vec2& origin_,
          const glm::vec2& size_):
    source(source_),
    index(index_),
    origin(origin_),
    size(size_)
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
FBOLayer::FBOLayer(FBOLayerSource source,
                   size_t index,
                   const glm::vec2& origin,
                   const glm::vec2& size):
  d(new Private(source, index, origin, size))
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
  size_t levels=1;
  switch(d->source)
  {
  case FBOLayerSource::CurrentDrawColor:
  {
    textureID = map()->currentDrawFBO().textureID;
    shader = static_cast<ImageShader*>(map()->getShader<ImageShader>());
    break;
  }

  case FBOLayerSource::CurrentDrawDepth:
  {
    textureID = map()->currentDrawFBO().depthID;
    shader = static_cast<ImageShader*>(map()->getShader<DepthImageShader>());
    break;
  }

  case FBOLayerSource::CurrentReadColor:
  {
    textureID = map()->currentReadFBO().textureID;
    shader = static_cast<ImageShader*>(map()->getShader<ImageShader>());
    break;
  }

  case FBOLayerSource::CurrentReadDepth:
  {
    textureID = map()->currentReadFBO().depthID;
    shader = static_cast<ImageShader*>(map()->getShader<DepthImageShader>());
    break;
  }

  case FBOLayerSource::LightColor:
  {
    if(d->index < map()->lightBuffers().size())
    {
      const auto& l = map()->lightBuffers().at(d->index);
      textureID = l.textureID;
      levels = l.levels;

      if(levels>1)
        shader = static_cast<ImageShader*>(map()->getShader<Image3DShader>());
      else
        shader = static_cast<ImageShader*>(map()->getShader<ImageShader>());
    }
    break;
  }

  case FBOLayerSource::LightDepth:
  {
    if(d->index < map()->lightBuffers().size())
    {
      const auto& l = map()->lightBuffers().at(d->index);
      textureID = l.depthID;
      levels = l.levels;

      if(levels>1)
        shader = static_cast<ImageShader*>(map()->getShader<DepthImage3DShader>());
      else
        shader = static_cast<ImageShader*>(map()->getShader<DepthImageShader>());
    }
    break;
  }
  }

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
  if(levels == 1)
    shader->setTexture(textureID);
  else
    shader->setTexture3D(textureID, 0);

  shader->draw(GL_TRIANGLE_FAN, d->vertexBuffer, glm::vec4(1.0f));
}

//##################################################################################################
void FBOLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
}

}
