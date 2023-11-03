#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"
#include "tp_maps/TexturePool.h"
#include "tp_maps/shaders/G3DMaterialShader.h"
#include "tp_maps/shaders/G3DImageShader.h"
#include "tp_maps/shaders/G3DXYZShader.h"
#include "tp_maps/shaders/G3DDepthShader.h"
#include "tp_maps/shaders/G3DStaticLightShader.h"

#include "tp_utils/TimeUtils.h"

namespace tp_maps
{

//##################################################################################################
struct Geometry3DLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::Geometry3DLayer::Private");
  TP_NONCOPYABLE(Private);

  Geometry3DLayer* q;

  tp_utils::StringID name{defaultSID()};
  bool geometrySet{false};

  std::vector<tp_utils::StringID> subscribedTextures;

  std::unique_ptr<Geometry3DPool> localGeometry3DPool;

  Geometry3DPool* geometry3DPool;


  ShaderSelection shaderSelection{ShaderSelection::Material};
  std::unordered_map<tp_utils::StringID, tp_utils::StringID> alternativeMaterials;

  std::vector<tp_math_utils::UVTransformation> uvTransformations;
  std::vector<glm::mat3> uvMatricies;

  //################################################################################################
  Private(Geometry3DLayer* q_, Geometry3DPool* geometry3DPool_):
    q(q_),
    geometry3DPool(geometry3DPool_)
  {
    if(!geometry3DPool)
    {
      localGeometry3DPool = std::make_unique<Geometry3DPool>(q);
      geometry3DPool = localGeometry3DPool.get();
    }

    geometry3DPoolChanged.connect(geometry3DPool->changed);
  }

  //################################################################################################
  ~Private()
  {
    for(const auto& name : subscribedTextures)
      geometry3DPool->texturePool()->unsubscribe(name);
  }

  //################################################################################################
  tp_utils::Callback<void()> geometry3DPoolChanged = [&]
  {
    TP_FUNCTION_TIME("Geometry3DLayer::geometry3DPoolChanged");
    q->update();
  };

  //################################################################################################
  void checkClearGeometry()
  {
    TP_FUNCTION_TIME("Geometry3DLayer::checkClearGeometry");
    if(geometrySet)
    {
      geometrySet=false;
      geometry3DPool->unsubscribe(name);
    }
  }
};

//##################################################################################################
Geometry3DLayer::Geometry3DLayer(Geometry3DPool* geometry3DPool):
  d(new Private(this, geometry3DPool))
{

}

//##################################################################################################
Geometry3DLayer::~Geometry3DLayer()
{
  delete d;
}

//##################################################################################################
void Geometry3DLayer::setName(const tp_utils::StringID& name)
{
  d->name = name;
  update();
}

//##################################################################################################
const tp_utils::StringID& Geometry3DLayer::name() const
{
  return d->name;
}

//##################################################################################################
TexturePool* Geometry3DLayer::texturePool() const
{
  return d->geometry3DPool->texturePool();
}

//##################################################################################################
void Geometry3DLayer::setTextures(const std::unordered_map<tp_utils::StringID, tp_image_utils::ColorMap>& textures)
{
  TP_FUNCTION_TIME("Geometry3DLayer::setTextures");
  std::vector<tp_utils::StringID> subscribedTextures;
  subscribedTextures.reserve(textures.size());
  for(const auto& i : textures)
  {
    d->geometry3DPool->texturePool()->subscribe(i.first, i.second, NChannels::RGBA);
    subscribedTextures.push_back(i.first);
  }

  d->subscribedTextures.swap(subscribedTextures);
  for(const auto& name : subscribedTextures)
    d->geometry3DPool->texturePool()->unsubscribe(name);
}

//##################################################################################################
Geometry3DPool* Geometry3DLayer::geometry3DPool() const
{
  return d->geometry3DPool;
}

//##################################################################################################
void Geometry3DLayer::setGeometry(const std::vector<tp_math_utils::Geometry3D>& geometry)
{
  TP_FUNCTION_TIME("Geometry3DLayer::setGeometry");
  d->checkClearGeometry();
  d->geometrySet = true;
  d->geometry3DPool->subscribe(d->name, [&]{return geometry;}, true);
}

//################################################################################################
void Geometry3DLayer::viewGeometry(const std::function<void(const std::vector<tp_math_utils::Geometry3D>&)>& closure) const
{
  d->geometry3DPool->viewGeometry(d->name, closure);
}

//################################################################################################
void Geometry3DLayer::viewGeometry(const std::function<void(const std::vector<tp_math_utils::Geometry3D>&, const std::vector<tp_math_utils::Material>&)>& closure) const
{
  d->geometry3DPool->viewGeometry(d->name, d->alternativeMaterials, closure);
}

//##################################################################################################
void Geometry3DLayer::setShaderSelection(ShaderSelection shaderSelection)
{
  d->shaderSelection = shaderSelection;
  update();
}

//##################################################################################################
Geometry3DLayer::ShaderSelection Geometry3DLayer::shaderSelection() const
{
  return d->shaderSelection;
}

//################################################################################################
void Geometry3DLayer::setAlternativeMaterials(const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& alternativeMaterials)
{
  d->alternativeMaterials = alternativeMaterials;
  update();
}

//################################################################################################
const std::unordered_map<tp_utils::StringID, tp_utils::StringID>& Geometry3DLayer::alternativeMaterials() const
{
  return d->alternativeMaterials;
}

//##################################################################################################
void Geometry3DLayer::setUVTransformations(const std::vector<tp_math_utils::UVTransformation>& uvTransformations)
{
  d->uvTransformations = uvTransformations;
  d->uvMatricies.resize(d->uvTransformations.size());
  for(size_t i=0; i<d->uvTransformations.size(); i++)
    d->uvMatricies[i] = d->uvTransformations.at(i).uvMatrix();
  update();
}

//##################################################################################################
const std::vector<tp_math_utils::UVTransformation>& Geometry3DLayer::uvTransformations() const
{
  return d->uvTransformations;
}

//##################################################################################################
void Geometry3DLayer::render(RenderInfo& renderInfo)
{
  TP_FUNCTION_TIME("Geometry3DLayer::render");

  if(renderInfo.pass != defaultRenderPass().type &&
     renderInfo.pass != RenderPass::Transparency &&
     renderInfo.pass != RenderPass::LightFBOs &&
     renderInfo.pass != RenderPass::Picking)
    return;

  Matrices m;
  if(renderInfo.pass == RenderPass::LightFBOs)
    m = map()->controller()->lightMatrices();
  else
    m = map()->controller()->matrices(coordinateSystem());

  Geometry3DShader* shader{nullptr};
  switch(d->shaderSelection)
  {
  case ShaderSelection::Material    : shader = map()->getShader<G3DMaterialShader>   (); break;
  case ShaderSelection::Image       : shader = map()->getShader<G3DImageShader>      (); break;
  case ShaderSelection::XYZ         : shader = map()->getShader<G3DXYZShader>        (); break;
  case ShaderSelection::Depth       : shader = map()->getShader<G3DDepthShader>      (); break;
  case ShaderSelection::StaticLight : shader = map()->getShader<G3DStaticLightShader>(); break;
  }

  if(!shader || shader->error())
    return;

  shader->initPass(renderInfo, m, modelToWorldMatrix());

  if(renderInfo.pass == RenderPass::Picking)
  {
    d->geometry3DPool->viewProcessedGeometry(d->name,
                                             shader,
                                             d->alternativeMaterials,
                                             d->uvMatricies,
                                             [&](const std::vector<ProcessedGeometry3D>& processedGeometry)
    {
      size_t iMax = processedGeometry.size();
      for(size_t i=0; i<iMax; i++)
      {
        const auto& details = processedGeometry.at(i);
        auto pickingID = renderInfo.pickingIDMat(PickingDetails(i, [&](const PickingResult& r)
        {
          return new GeometryPickingResult(r.pickingType, r.details, r.renderInfo, this);
        }));

        shader->setMaterialPicking(renderInfo, details);
        for(const std::pair<GLenum, G3DMaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
          shader->drawPicking(renderInfo, details, buff.first, buff.second, pickingID);
      }
    });
  }
  else
  {
    d->geometry3DPool->viewProcessedGeometry(d->name,
                                             shader,
                                             d->alternativeMaterials,
                                             d->uvMatricies,
                                             [&](const std::vector<ProcessedGeometry3D>& processedGeometry)
    {
      for(const auto& details : processedGeometry)
      {
        shader->setMaterial(renderInfo, details);
        for(const std::pair<GLenum, G3DMaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
          shader->draw(renderInfo, details, buff.first, buff.second);
      }
    });
  }
}

}
