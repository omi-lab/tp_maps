#include "tp_maps/layers/FBOLayer.h"
#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/shaders/DepthImageShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"

#include "tp_math_utils/JSONUtils.h"

#include "glm/gtc/type_ptr.hpp" // IWYU pragma: keep

#include <vector>

namespace tp_maps
{

//##################################################################################################
std::string fboLayerSourceToString(FBOLayerSource fboLayerSource)
{
  switch(fboLayerSource)
  {
  case FBOLayerSource::CurrentReadColor : return "CurrentReadColor";
  case FBOLayerSource::CurrentReadDepth : return "CurrentReadDepth";
  case FBOLayerSource::CurrentDrawColor : return "CurrentDrawColor";
  case FBOLayerSource::CurrentDrawDepth : return "CurrentDrawDepth";
  case FBOLayerSource::LightColor       : return "LightColor";
  case FBOLayerSource::LightDepth       : return "LightDepth";
  }

  return "CurrentReadColor";
}

//##################################################################################################
FBOLayerSource fboLayerSourceFromString(const std::string& fboLayerSource)
{
  if(fboLayerSource == "CurrentReadColor")return FBOLayerSource::CurrentReadColor;
  if(fboLayerSource == "CurrentReadDepth")return FBOLayerSource::CurrentReadDepth;
  if(fboLayerSource == "CurrentDrawColor")return FBOLayerSource::CurrentDrawColor;
  if(fboLayerSource == "CurrentDrawDepth")return FBOLayerSource::CurrentDrawDepth;
  if(fboLayerSource == "LightColor"      )return FBOLayerSource::LightColor      ;
  if(fboLayerSource == "LightDepth"      )return FBOLayerSource::LightDepth      ;
  return FBOLayerSource::CurrentReadColor;
}

//##################################################################################################
std::vector<std::string> fboLayerSources()
{
  return
  {
    "CurrentReadColor",
    "CurrentReadDepth",
    "CurrentDrawColor",
    "CurrentDrawDepth",
    "LightColor"      ,
    "LightDepth"      ,
  };
}

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
const glm::vec2& FBOLayer::origin() const
{
  return d->origin;
}

//##################################################################################################
const glm::vec2& FBOLayer::size() const
{
  return d->size;
}

//##################################################################################################
void FBOLayer::setSource(FBOLayerSource source, size_t index)
{
  d->source = source;
  d->index = index;
  update();
}

//##################################################################################################
FBOLayerSource FBOLayer::source() const
{
  return d->source;
}

//##################################################################################################
size_t FBOLayer::index() const
{
  return d->index;
}

//##################################################################################################
nlohmann::json FBOLayer::saveState() const
{
  nlohmann::json j;

  j["source"] = fboLayerSourceToString(d->source);
  j["index"] = d->index;

  j["origin"] = tp_math_utils::vec2ToJSON(d->origin);
  j["size"  ] = tp_math_utils::vec2ToJSON(d->size  );

  return j;
}

//##################################################################################################
void FBOLayer::loadState(const nlohmann::json& j)
{
  d->source = fboLayerSourceFromString(TPJSONString(j, "source"));
  d->index = TPJSONSizeT(j, "index");

  d->origin = TPJSONVec2(j, "origin", d->origin);
  d->size = TPJSONVec2(j, "size", d->size);
}

//##################################################################################################
void FBOLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass().type)
    return;

  GLuint textureID{0};
  ImageShader* shader{nullptr};
  size_t levels=1;
  switch(d->source)
  {
  case FBOLayerSource::CurrentDrawColor:
  {
    textureID = map()->currentDrawFBO().textureID;
    shader = map()->getShader<ImageShader>();
    break;
  }

  case FBOLayerSource::CurrentDrawDepth:
  {
    textureID = map()->currentDrawFBO().depthID;
    shader = map()->getShader<DepthImageShader>();
    break;
  }

  case FBOLayerSource::CurrentReadColor:
  {
    textureID = map()->currentReadFBO().textureID;
    shader = map()->getShader<ImageShader>();
    break;
  }

  case FBOLayerSource::CurrentReadDepth:
  {
    textureID = map()->currentReadFBO().depthID;
    shader = map()->getShader<DepthImageShader>();
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
        shader = map()->getShader<Image3DShader>();
      else
        shader = map()->getShader<ImageShader>();
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
        shader = map()->getShader<DepthImage3DShader>();
      else
        shader = map()->getShader<DepthImageShader>();
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
    verts.push_back(ImageShader::Vertex(topRight   , {0,0,1}, {1.0f, 1.0f}));
    verts.push_back(ImageShader::Vertex(bottomRight, {0,0,1}, {1.0f, 0.0f}));
    verts.push_back(ImageShader::Vertex(bottomLeft , {0,0,1}, {0.0f, 0.0f}));
    verts.push_back(ImageShader::Vertex(topLeft    , {0,0,1}, {0.0f, 1.0f}));

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

  glScissor(0, 0, map()->width(), map()->height());

  shader->draw(GL_TRIANGLE_FAN, d->vertexBuffer, glm::vec4(1.0f));
}

//##################################################################################################
void FBOLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  Layer::invalidateBuffers();
}

}
