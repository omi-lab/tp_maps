#include "tp_maps/layers/FBOLayer.h"
#include "tp_maps/shaders/G3DImageShader.h"
#include "tp_maps/shaders/G3DDepthImageShader.h"
#include "tp_maps/shaders/G3DPickingShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/PickingResult.h"
#include "tp_maps/subsystems/open_gl/OpenGLBuffers.h" // IWYU pragma: keep

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
    case FBOLayerSource::Color    : return "Color";
    case FBOLayerSource::Depth    : return "Depth";
    case FBOLayerSource::Normals  : return "Normals";
    case FBOLayerSource::Specular : return "Specular";
    case FBOLayerSource::Picking  : return "Picking";
  }

  return "Color";
}

//##################################################################################################
FBOLayerSource fboLayerSourceFromString(const std::string& fboLayerSource)
{
  if(fboLayerSource == "Color"   ) return FBOLayerSource::Color;
  if(fboLayerSource == "Depth"   ) return FBOLayerSource::Depth;
  if(fboLayerSource == "Normals" ) return FBOLayerSource::Normals;
  if(fboLayerSource == "Specular") return FBOLayerSource::Specular;
  if(fboLayerSource == "Picking" ) return FBOLayerSource::Picking;
  return FBOLayerSource::Color;
}

//##################################################################################################
std::vector<std::string> fboLayerSources()
{
  return
  {
    "Color"   ,
    "Depth"   ,
    "Normals" ,
    "Specular",
    "Picking"
  };
}

//##################################################################################################
void FBOWindow::saveState(nlohmann::json& j) const
{
  j["fboName"] = fboName.toString();
  j["source"]  = fboLayerSourceToString(source);
  j["origin"]  = tp_math_utils::vec2ToJSON(origin);
  j["size"  ]  = tp_math_utils::vec2ToJSON(size);
}

//##################################################################################################
void FBOWindow::loadState(const nlohmann::json& j)
{
  fboName = TPJSONString(j, "fboName");
  source  = fboLayerSourceFromString(TPJSONString(j, "source"));
  origin  = TPJSONVec2(j, "origin", origin);
  size    = TPJSONVec2(j, "size", size);
}

//##################################################################################################
struct FBOLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::FBOLayer::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  std::vector<FBOWindow> windows;

  bool pollPicking{false};
  bool actuallyPollPicking{false};
  double updateAfterTimestampMS{0.0};

  std::vector<G3DImageShader::VertexBuffer*> vertexBuffers;
  bool updateVertexBuffer{true};

  //################################################################################################
  Private(Q* q_):
    q(q_)
  {

  }

  //################################################################################################
  void updatePollPicking()
  {
    actuallyPollPicking = false;
    if(!pollPicking)
      return;

    for(const auto& window : windows)
    {
      if(window.fboName == "renderToImage")
      {
        actuallyPollPicking = true;
        break;
      }
    }
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    tpDeleteAll(vertexBuffers);
    vertexBuffers.clear();
  }

  //################################################################################################
  G3DImageShader* findShader(const FBOWindow& window)
  {
    switch(window.source)
    {
      case FBOLayerSource::Color   : return q->map()->getShader<G3DImageShader>();
      case FBOLayerSource::Depth   : return q->map()->getShader<G3DDepthImageShader>();
      case FBOLayerSource::Normals : return q->map()->getShader<G3DImageShader>();
      case FBOLayerSource::Specular: return q->map()->getShader<G3DImageShader>();
      case FBOLayerSource::Picking : return q->map()->getShader<G3DPickingShader>();
    }
    return q->map()->getShader<G3DImageShader>();
  }
};

//##################################################################################################
FBOLayer::FBOLayer():
  d(new Private(this))
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
void FBOLayer::setWindows(const std::vector<FBOWindow>& windows)
{
  d->windows = windows;
  d->updateVertexBuffer = true;
  d->updatePollPicking();
  update();
}

//##################################################################################################
const std::vector<FBOWindow>& FBOLayer::windows() const
{
  return d->windows;
}

//##################################################################################################
void FBOLayer::setPollPicking(bool pollPicking)
{
  d->pollPicking = pollPicking;
  d->updatePollPicking();
}

//##################################################################################################
bool FBOLayer::pollPicking() const
{
  return d->pollPicking;
}

//##################################################################################################
void FBOLayer::saveState(nlohmann::json& j) const
{
  j["pollPicking"] = d->pollPicking;

  auto& windowsJ = j["windows"];
  windowsJ = nlohmann::json::array();
  windowsJ.get_ptr<nlohmann::json::array_t*>()->reserve(d->windows.size());
  for(const auto& window : d->windows)
  {
    windowsJ.emplace_back();
    window.saveState(windowsJ.back());
  }
}

//##################################################################################################
void FBOLayer::loadState(const nlohmann::json& j)
{
  d->pollPicking = TPJSONBool(j, "pollPicking", false);
  d->windows.clear();
  if(const auto i=j.find("windows"); i!=j.end() && i->is_array())
  {
    d->windows.reserve(i->size());
    for(const auto& jj : *i)
      d->windows.emplace_back().loadState(jj);
  }
  d->updatePollPicking();
}

//##################################################################################################
void FBOLayer::render(RenderInfo& renderInfo)
{
  if(renderInfo.pass != defaultRenderPass().type)
    return;

  if(d->updateVertexBuffer)
  {
    d->deleteVertexBuffers();

    d->vertexBuffers.reserve(d->windows.size());
    for(const auto& window : d->windows)
    {
      auto w = window.size.x;
      auto h = window.size.y;

      auto topRight    = glm::vec3(window.origin + glm::vec2(w, 0), 0.0f);
      auto bottomRight = glm::vec3(window.origin + glm::vec2(w, h), 0.0f);
      auto bottomLeft  = glm::vec3(window.origin + glm::vec2(0, h), 0.0f);
      auto topLeft     = glm::vec3(window.origin + glm::vec2(0, 0), 0.0f);

      std::vector<GLuint> indexes{3,2,1,0};
      std::vector<G3DImageShader::Vertex> verts;

      verts.clear();

      //                                     vertex         tbnq       texture
      verts.push_back(G3DImageShader::Vertex(topRight   , {0,0,0,1}, {1.0f, 1.0f}));
      verts.push_back(G3DImageShader::Vertex(bottomRight, {0,0,0,1}, {1.0f, 0.0f}));
      verts.push_back(G3DImageShader::Vertex(bottomLeft , {0,0,0,1}, {0.0f, 0.0f}));
      verts.push_back(G3DImageShader::Vertex(topLeft    , {0,0,0,1}, {0.0f, 1.0f}));

      auto shader = d->findShader(window);

      if(!shader || shader->error())
        return;

      d->vertexBuffers.push_back(shader->generateVertexBuffer(map(), indexes, verts));
    }

    d->updateVertexBuffer=false;
  }

  if(d->windows.size() != d->vertexBuffers.size())
    return;

  for(size_t i=0; i<d->windows.size(); i++)
  {
    const auto& window = d->windows.at(i);
    auto vertexBuffer = d->vertexBuffers.at(i);

    OpenGLFBO* fbo = tpGetMapValue(map()->buffers().storedBuffers(), window.fboName, nullptr);
//    if(!fbo)
//    {
//      fbo = tpGetMapValue(map()->intermediateBuffers(), window.fboName, nullptr);
      if(!fbo)
        continue;
//    }

    GLuint textureID{0};
    switch(window.source)
    {
      case FBOLayerSource::Color:
      {
        textureID = fbo->textureID;
        break;
      }

      case FBOLayerSource::Depth:
      {
        textureID = fbo->depthID;
        break;
      }

      case FBOLayerSource::Normals:
      {
        textureID = fbo->normalsID;
        break;
      }

      case FBOLayerSource::Specular:
      {
        textureID = fbo->specularID;
        break;
      }

      case FBOLayerSource::Picking:
      {
        textureID = fbo->textureID;
        break;
      }
    }

    G3DImageShader* shader = d->findShader(window);

    if(!shader || shader->error())
      return;

    if(!textureID)
      return;

    glm::mat4 matrix(1.0f);
    matrix = glm::translate(matrix, {-1.0f, 1.0f, 0.0f});
    matrix = glm::scale(matrix, {2.0f, -2.0f, 1.0f});

    shader->use(renderInfo.shaderType());
    shader->setMatrix(matrix);
    shader->setTexture(textureID);

    glScissor(0, 0, map()->width(), map()->height());

    shader->draw(GL_TRIANGLE_FAN, vertexBuffer, glm::vec4(1.0f));
  }
}

//##################################################################################################
void FBOLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  Layer::invalidateBuffers();
}

//##################################################################################################
void FBOLayer::animate(double timestampMS)
{
  if(visible() && d->actuallyPollPicking && d->updateAfterTimestampMS<timestampMS)
  {
    d->updateAfterTimestampMS = timestampMS + 100;
    tp_maps::PickingResult* pickingResult = map()->performPicking("Poll", {10,10});
    TP_CLEANUP([&]{delete pickingResult;});
  }

  Layer::animate(timestampMS);
}

}
