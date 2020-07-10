#include "tp_maps/layers/ImageLayer.h"
#include "tp_maps/shaders/ImageShader.h"
#include "tp_maps/picking_results/ImagePickingResult.h"
#include "tp_maps/Texture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Controller.h"
#include "tp_maps/RenderInfo.h"

#include "tp_math_utils/Transformation.h"

#include "tp_utils/RefCount.h"
#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

#include <vector>

namespace tp_maps
{
//##################################################################################################
struct ImageLayer::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::ImageLayer::Private");
  TP_NONCOPYABLE(Private);

  ImageLayer* q;

  Texture* texture;

  //The raw data passed to this class
  std::vector<GLuint> indexes{0,1,2,3};
  std::vector<ImageShader::Vertex> verts;
  glm::vec4 color{1.0f, 1.0f, 1.0f, 1.0f};

  glm::vec3 topRight{};
  glm::vec3 bottomRight{};
  glm::vec3 bottomLeft{};
  glm::vec3 topLeft{};

  ImageShader::VertexBuffer* vertexBuffer{nullptr};

  GLuint textureID{0};

  bool bindBeforeRender{true};
  bool externalCoords{false};
  bool updateVertexBuffer{true};

  std::function<ImageShader*(Map*)> getShader;

  //################################################################################################
  Private(ImageLayer* q_, Texture* texture_):
    q(q_),
    texture(texture_),
    getShader([](Map* map){return static_cast<ImageShader*>(map->getShader<ImageShader>());})
  {

  }

  //################################################################################################
  ~Private()
  {
    if(textureID)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(textureID);
    }

    delete texture;
    deleteVertexBuffers();
  }

  //################################################################################################
  void deleteVertexBuffers()
  {
    delete vertexBuffer;
    vertexBuffer=nullptr;
  }
};

//##################################################################################################
ImageLayer::ImageLayer(Texture* texture):
  d(new Private(this, texture))
{
  texture->setImageChangedCallback([this]()
  {
    d->bindBeforeRender = true;
    update();
  });
}

//##################################################################################################
ImageLayer::~ImageLayer()
{
  delete d;
}

//##################################################################################################
void ImageLayer::setColor(const glm::vec4& color)
{
  d->color = color;
}

//##################################################################################################
glm::vec4 ImageLayer::color()const
{
  return d->color;
}

//##################################################################################################
void ImageLayer::setImageCoords(const glm::vec3& topRight,
                                const glm::vec3& bottomRight,
                                const glm::vec3& bottomLeft,
                                const glm::vec3& topLeft)
{
  d->topRight    = topRight;
  d->bottomRight = bottomRight;
  d->bottomLeft  = bottomLeft;
  d->topLeft     = topLeft;
  d->externalCoords = true;
  d->updateVertexBuffer = true;
  update();
}

//##################################################################################################
void ImageLayer::setShader(const std::function<ImageShader*(Map*)>& getShader)
{
  d->getShader = getShader;
}

//##################################################################################################
void ImageLayer::render(RenderInfo& renderInfo)
{
  if(!d->texture->imageReady())
    return;

  if(renderInfo.pass != defaultRenderPass() && renderInfo.pass != RenderPass::Picking)
    return;

  auto shader = d->getShader(map());
  if(shader->error())
    return;

  if(d->bindBeforeRender)
  {
    d->bindBeforeRender=false;
    map()->deleteTexture(d->textureID);
    d->textureID = d->texture->bindTexture();
    d->updateVertexBuffer=true;
  }

  if(!d->textureID)
    return;

  if(d->updateVertexBuffer)
  {
    d->verts.clear();

    glm::vec2 t = d->texture->textureDims();

    if(d->externalCoords)
    {
      d->verts.push_back(ImageShader::Vertex(d->topRight   , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { t.x,  t.y}));
      d->verts.push_back(ImageShader::Vertex(d->bottomRight, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { t.x, 0.0f}));
      d->verts.push_back(ImageShader::Vertex(d->bottomLeft , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}));
      d->verts.push_back(ImageShader::Vertex(d->topLeft    , {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f,  t.y}));
    }
    else
    {
      glm::vec2 i = d->texture->imageDims();

      float w = i.x;
      float h = i.y;
      float x = 0.0f;
      float y = 0.0f;

      d->verts.push_back(ImageShader::Vertex({w,y,0}, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { t.x,  t.y}));
      d->verts.push_back(ImageShader::Vertex({w,h,0}, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, { t.x, 0.0f}));
      d->verts.push_back(ImageShader::Vertex({x,h,0}, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}));
      d->verts.push_back(ImageShader::Vertex({x,y,0}, {0,0,1}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f,  t.y}));
    }

    delete d->vertexBuffer;
    d->vertexBuffer = shader->generateVertexBuffer(map(), d->indexes, d->verts);
    d->updateVertexBuffer=false;
  }

  shader->use();
  shader->setMatrix(map()->controller()->matrix(coordinateSystem()));
  shader->setTexture(d->textureID);

  map()->controller()->enableScissor(coordinateSystem());
  if(renderInfo.pass==RenderPass::Picking)
  {
    auto pickingID = renderInfo.pickingIDMat(PickingDetails(0, [](const PickingResult& r)
    {
      return new ImagePickingResult(r.pickingType, r.details, r.renderInfo, 0, 0);
    }));
    shader->drawPicking(GL_TRIANGLE_FAN,
                             d->vertexBuffer,
                             pickingID);
  }
  else
  {
    shader->draw(GL_TRIANGLE_FAN,
                      d->vertexBuffer,
                      d->color);
  }
  map()->controller()->disableScissor();
}

//##################################################################################################
void ImageLayer::invalidateBuffers()
{
  d->deleteVertexBuffers();
  d->updateVertexBuffer=true;
  d->textureID = 0;
  d->bindBeforeRender = true;
}

}
