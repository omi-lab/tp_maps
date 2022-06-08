#include "tp_maps/layers/Geometry3DLayer.h"
#include "tp_maps/Geometry3DPool.h"
#include "tp_maps/Texture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/picking_results/GeometryPickingResult.h"
#include "tp_maps/TexturePool.h"

#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/shaders/ImageShader.h"

#include "tp_utils/DebugUtils.h"
#include "tp_utils/TimeUtils.h"

#include "tp_triangulation/Triangulation.h"

#include "glm/glm.hpp"

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
    TP_TIME_SCOPE("Geometry3DLayer::geometry3DPoolChanged");
    q->update();
  };

  //################################################################################################
  void checkClearGeometry()
  {
    TP_TIME_SCOPE("Geometry3DLayer::checkClearGeometry");
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
  TP_TIME_SCOPE("Geometry3DLayer::setTextures");
  std::vector<tp_utils::StringID> subscribedTextures;
  subscribedTextures.reserve(textures.size());
  for(const auto& i : textures)
  {
    d->geometry3DPool->texturePool()->subscribe(i.first, i.second);
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
  TP_TIME_SCOPE("Geometry3DLayer::setGeometry");
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
void Geometry3DLayer::render(RenderInfo& renderInfo)
{
  TP_TIME_SCOPE("Geometry3DLayer::render");
  if(renderInfo.pass != defaultRenderPass() &&
     renderInfo.pass != RenderPass::Transparency &&
     renderInfo.pass != RenderPass::LightFBOs &&
     renderInfo.pass != RenderPass::Picking)
    return;

  //== Get matrices ================================================================================
  Matrices m;
  if(renderInfo.pass == RenderPass::LightFBOs)
    m = map()->controller()->lightMatrices();
  else
    m = map()->controller()->matrices(coordinateSystem());

  //== Common ======================================================================================
  auto render = [&](auto s,
      const auto& use,
      const auto& setMaterialPicking,
      const auto& drawPicking,
      const auto& setMaterial,
      const auto& draw,
      RenderInfo& renderInfo)
  {
    auto shader = map()->getShader<typename std::remove_pointer<decltype(s)>::type>();
    if(shader->error())
      return;

    use(shader);

    if(renderInfo.pass == RenderPass::Picking)
    {
      d->geometry3DPool->viewProcessedGeometry(d->name, shader, {}, [&](const std::vector<ProcessedGeometry3D>& processedGeometry)
      {
        size_t iMax = processedGeometry.size();
        for(size_t i=0; i<iMax; i++)
        {
          const auto& details = processedGeometry.at(i);
          auto pickingID = renderInfo.pickingIDMat(PickingDetails(i, [&](const PickingResult& r)
          {
            return new GeometryPickingResult(r.pickingType, r.details, r.renderInfo, this);
          }));

          setMaterialPicking(shader, details);
          for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
            drawPicking(shader, details, buff.first, buff.second, pickingID);
        }
      });
    }
    else
    {
      d->geometry3DPool->viewProcessedGeometry(d->name, shader, d->alternativeMaterials, [&](const std::vector<ProcessedGeometry3D>& processedGeometry)
      {
        for(const auto& details : processedGeometry)
        {
          setMaterial(shader, details);
          for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
            draw(shader, details, buff.first, buff.second);
        }
      });
    }
  };

  //== MaterialShader ==============================================================================
  if(d->shaderSelection == ShaderSelection::Material)
  {
    render(static_cast<MaterialShader*>(nullptr), [&](auto shader) //-- use ------------------------
    {
      shader->use(renderInfo.shaderType());
      shader->setMatrix(modelToWorldMatrix(), m.v, m.p);
      shader->setLights(map()->lights(), map()->lightBuffers());
      shader->setLightOffsets(map()->renderedLightLevels());
    },
    [&](auto, const auto&) //-- setMaterialPicking -------------------------------------------------
    {

    },
    [&](auto shader, const auto& details, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      if(details.alternativeMaterial->material.rayVisibilityShadowCatcher)
        return;
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setMaterial(details.alternativeMaterial->material);
      shader->setTextures(details.alternativeMaterial->rgbaTextureID,
                          details.alternativeMaterial->normalsTextureID,
                          details.alternativeMaterial->rmttrTextureID);
      shader->setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
    },
    [&](auto shader, const auto& details, GLenum mode, Geometry3DShader::VertexBuffer* vertexBuffer) //-- draw ----------
    {
      if(renderInfo.pass == RenderPass::LightFBOs && details.alternativeMaterial->material.rayVisibilityShadowCatcher)
        return;
      shader->draw(mode, vertexBuffer);
    },
    renderInfo);
  }

  //== ImageShader =================================================================================
  else if(d->shaderSelection == ShaderSelection::Image)
  {
    render(static_cast<ImageShader*>(nullptr), [&](auto shader) //-- use ---------------------------
    {
      shader->use();
      shader->setMatrix(m.vp * modelToWorldMatrix());
    },
    [&](auto shader, const auto& details) //-- setMaterialPicking ----------------------------------
    {
      shader->setTexture(details.rgbaTextureID);
    },
    [&](auto shader, const auto& details, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setTexture(details.rgbaTextureID);
    },
    [&](auto shader, const auto& details, auto first, auto second) //-- draw --------------------------------------------
    {
      shader->draw(first, second, {1.0f, 1.0f, 1.0f, 1.0f});
    },
    renderInfo);
  }
}

}
