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

  Geometry3DPool localGeometry3DPool;
  Geometry3DPool* geometry3DPool{&localGeometry3DPool};

  TexturePool localTexturePool;
  TexturePool* texturePool{&localTexturePool};

  ShaderSelection shaderSelection{ShaderSelection::Material};
  std::unordered_map<tp_utils::StringID, tp_utils::StringID> alternativeMaterials;

  //################################################################################################
  Private(Geometry3DLayer* q_):
    q(q_),
    localGeometry3DPool(q),
    localTexturePool(q)
  {
    geometry3DPool->setTexturePool(texturePool);
    geometry3DPoolChanged.connect(geometry3DPool->changedCallbacks);
  }

  //################################################################################################
  ~Private()
  {
    for(const auto& name : subscribedTextures)
      texturePool->unsubscribe(name);
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
Geometry3DLayer::Geometry3DLayer():
  d(new Private(this))
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
void Geometry3DLayer::setTexturePool(TexturePool* texturePool)
{  
  TP_TIME_SCOPE("Geometry3DLayer::setTexturePool");
  for(const auto& name : d->subscribedTextures)
    d->texturePool->unsubscribe(name);
  d->subscribedTextures.clear();

  d->texturePool = texturePool;
  d->localGeometry3DPool.setTexturePool(d->texturePool);
  update();
}

//##################################################################################################
TexturePool* Geometry3DLayer::texturePool() const
{
  return d->texturePool;
}

//##################################################################################################
void Geometry3DLayer::setTextures(const std::unordered_map<tp_utils::StringID, tp_image_utils::ColorMap>& textures)
{
  TP_TIME_SCOPE("Geometry3DLayer::setTextures");
  std::vector<tp_utils::StringID> subscribedTextures;

  for(const auto& i : textures)
  {
    d->texturePool->subscribe(i.first, i.second);
    subscribedTextures.push_back(i.first);
  }

  d->subscribedTextures.swap(subscribedTextures);
  for(const auto& name : subscribedTextures)
    d->texturePool->unsubscribe(name);
}

//##################################################################################################
void Geometry3DLayer::setGeometry3DPool(Geometry3DPool* geometry3DPool)
{
  TP_TIME_SCOPE("Geometry3DLayer::setGeometry3DPool");
  d->checkClearGeometry();
  d->geometry3DPool = geometry3DPool;
  d->geometry3DPoolChanged.disconnect();
  d->geometry3DPoolChanged.connect(d->geometry3DPool->changedCallbacks);
  d->geometry3DPoolChanged();
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
            drawPicking(shader, buff.first, buff.second, pickingID);
        }
      });
    }
    else
    {
      if(d->name == "1e7ab311-87a6-4177-87c8-e04f71b0dbc9")
        return;
      d->geometry3DPool->viewProcessedGeometry(d->name, shader, d->alternativeMaterials, [&](const std::vector<ProcessedGeometry3D>& processedGeometry)
      {
        for(const auto& details : processedGeometry)
        {
          setMaterial(shader, details);
          for(const std::pair<GLenum, MaterialShader::VertexBuffer*>& buff : details.vertexBuffers)
            draw(shader, buff.first, buff.second);
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
    },
    [&](auto, const auto&) //-- setMaterialPicking -------------------------------------------------
    {

    },
    [&](auto shader, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setMaterial(details.alternativeMaterial->material);
      shader->setTextures(details.alternativeMaterial->rgbaTextureID,
                          details.alternativeMaterial->specularTextureID,
                          details.alternativeMaterial->normalsTextureID,
                          details.alternativeMaterial->rmaoTextureID,
                          map()->spotLightTexture());

      shader->setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
    },
    [&](auto shader, auto first, auto second) //-- draw --------------------------------------------
    {
      shader->draw(first, second);
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
    [&](auto shader, auto first, auto second, auto pickingID) //-- drawPicking ---------------------
    {
      shader->drawPicking(first, second, pickingID);
    },
    [&](auto shader, const auto& details) //-- setMaterial -----------------------------------------
    {
      shader->setTexture(details.rgbaTextureID);
    },
    [&](auto shader, auto first, auto second) //-- draw --------------------------------------------
    {
      shader->draw(first, second, {1.0f, 1.0f, 1.0f, 1.0f});
    },
    renderInfo);
  }
}

}
