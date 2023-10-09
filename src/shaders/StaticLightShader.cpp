#include "tp_maps/shaders/StaticLightShader.h"
#include "tp_maps/RenderInfo.h"
#include "tp_maps/textures/BasicTexture.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_math_utils/Material.h"

#include "tp_utils/TimeUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderResource& vertShaderStr()       {static ShaderResource s{"/tp_maps/StaticLightShader.render.vert"};  return s;}
ShaderResource& fragShaderStr()       {static ShaderResource s{"/tp_maps/StaticLightShader.render.frag"};  return s;}
ShaderResource& vertShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.vert"}; return s;}
ShaderResource& fragShaderStrPicking(){static ShaderResource s{"/tp_maps/MaterialShader.picking.frag"}; return s;}

//##################################################################################################
struct UniformLocations_lt
{
  GLint                             mMatrixLocation{0};
  GLint                            mvMatrixLocation{0};
  GLint                           mvpMatrixLocation{0};
  GLint                             vMatrixLocation{0};
  GLint                          mInvMatrixLocation{0};
  GLint                            uvMatrixLocation{0};

  GLint                        cameraOriginLocation{0};

  GLint                  materialUseAmbientLocation{0};
  GLint                  materialUseDiffuseLocation{0};
  GLint                    materialUseNdotLLocation{0};
  GLint              materialUseAttenuationLocation{0};
  GLint                   materialUseShadowLocation{0};
  GLint                materialUseLightMaskLocation{0};
  GLint               materialUseReflectionLocation{0};

  GLint               materialShadowCatcherLocation{0};

  GLint                 materialAlbedoScaleLocation{0};

  GLint            materialAlbedoBrightnessLocation{0};
  GLint              materialAlbedoContrastLocation{0};
  GLint                 materialAlbedoGammaLocation{0};
  GLint                   materialAlbedoHueLocation{0};
  GLint            materialAlbedoSaturationLocation{0};
  GLint                 materialAlbedoValueLocation{0};
  GLint                materialAlbedoFactorLocation{0};

  GLint                             txlSizeLocation{0};
  GLint                      discardOpacityLocation{0};
  GLint                        lightOffsetsLocation{0};

  GLint                         rgbaTextureLocation{0};
  GLint                      normalsTextureLocation{0};
  GLint                        rmttrTextureLocation{0};
};

}

//##################################################################################################
struct StaticLightShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_math_utils::StaticLightShader::Private");
  TP_NONCOPYABLE(Private);

  StaticLightShader* q;

  ShaderType shaderType{ShaderType::Render};

  size_t maxLights{1};

  UniformLocations_lt renderLocations;
  UniformLocations_lt renderHDRLocations;

  GLint  pickingMVPMatrixLocation{0};
  GLint         pickingIDLocation{0};

  GLuint emptyTextureID{0};
  GLuint emptyNormalTextureID{0};

  //################################################################################################
  Private(StaticLightShader* q_):
    q(q_)
  {

  }

  //################################################################################################
  ~Private()
  {
    if(q->map() && emptyTextureID)
    {
      q->map()->makeCurrent();
      q->map()->deleteTexture(emptyTextureID);
      q->map()->deleteTexture(emptyNormalTextureID);
    }
  }

  //################################################################################################
  void draw(GLenum mode, StaticLightShader::VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }

  //################################################################################################
  void compile(ShaderResource& vertStr,
               ShaderResource& fragStr,
               ShaderType shaderType)
  {
    const char* vertShaderStr = vertStr.data(q->openGLProfile(), shaderType);
    const char* fragShaderStr = fragStr.data(q->openGLProfile(), shaderType);

    q->compile(vertShaderStr,
               fragShaderStr,
               [&](GLuint program)
    {
      switch(shaderType)
      {
      case ShaderType::Render: [[fallthrough]];
      case ShaderType::RenderExtendedFBO:
      {
        glBindAttribLocation(program, 0, "inVertex");
        glBindAttribLocation(program, 1, "inNormal");
        glBindAttribLocation(program, 2, "inTexture");
        break;
      }

      case ShaderType::Picking:
      {
        glBindAttribLocation(program, 0, "inVertex");
        break;
      }
      case ShaderType::Light: break;
      }
    },
    [&](GLuint program)
    {
      auto exec = [&](UniformLocations_lt& locations)
      {
        auto loc = [](auto program, const auto* name){return glGetUniformLocation(program, name);};

        locations.mMatrixLocation                = loc(program, "m");
        locations.mvMatrixLocation               = loc(program, "mv");
        locations.mvpMatrixLocation              = loc(program, "mvp");
        locations.vMatrixLocation                = loc(program, "v");
        locations.mInvMatrixLocation             = loc(program, "mInv");
        locations.uvMatrixLocation               = loc(program, "uvMatrix");

        locations.cameraOriginLocation           = loc(program, "cameraOrigin_world");

        locations.    materialUseAmbientLocation = loc(program, "material.useAmbient"    );
        locations.    materialUseDiffuseLocation = loc(program, "material.useDiffuse"    );
        locations.      materialUseNdotLLocation = loc(program, "material.useNdotL"      );
        locations.materialUseAttenuationLocation = loc(program, "material.useAttenuation");
        locations.     materialUseShadowLocation = loc(program, "material.useShadow"     );
        locations.  materialUseLightMaskLocation = loc(program, "material.useLightMask"  );
        locations. materialUseReflectionLocation = loc(program, "material.useReflection" );

        locations.materialShadowCatcherLocation  = loc(program, "material.rayVisibilityShadowCatcher");

        locations.  materialAlbedoScaleLocation      = loc(program, "material.albedoScale"  );
        locations.  materialAlbedoBrightnessLocation = loc(program, "material.albedoBrightness"  );
        locations.  materialAlbedoContrastLocation   = loc(program, "material.albedoContrast"    );
        locations.  materialAlbedoGammaLocation      = loc(program, "material.albedoGamma"       );
        locations.  materialAlbedoHueLocation        = loc(program, "material.albedoHue"         );
        locations.  materialAlbedoSaturationLocation = loc(program, "material.albedoSaturation"  );
        locations.  materialAlbedoValueLocation      = loc(program, "material.albedoValue"       );
        locations.  materialAlbedoFactorLocation     = loc(program, "material.albedoFactor"      );

        locations.txlSizeLocation                = loc(program, "txlSize");
        locations.discardOpacityLocation         = loc(program, "discardOpacity");
        locations.lightOffsetsLocation           = loc(program, "lightOffsets");

        locations.     rgbaTextureLocation       = loc(program, "rgbaTexture"     );
        locations.  normalsTextureLocation       = loc(program, "normalsTexture"  );
        locations.    rmttrTextureLocation       = loc(program, "rmttrTexture"    );
      };

      switch(shaderType)
      {
      case ShaderType::Render:
      {
        exec(renderLocations);
        break;
      }

      case ShaderType::RenderExtendedFBO:
      {
        exec(renderHDRLocations);
        break;
      }

      case ShaderType::Picking:
      {
        pickingMVPMatrixLocation = glGetUniformLocation(program, "mvp");
        pickingIDLocation        = glGetUniformLocation(program, "pickingID");
        break;
      }

      case ShaderType::Light: break;
      }
    },
    shaderType);
  }
};

//##################################################################################################
StaticLightShader::StaticLightShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  Geometry3DShader(map, openGLProfile),
  d(new Private(this))
{
  TP_TIME_SCOPE("StaticLightShader::StaticLightShader");
  auto bindTexture = [&](const TPPixel& color)
  {
    tp_image_utils::ColorMap textureData{1, 1, nullptr, color};
    BasicTexture texture(map, textureData);
    return texture.bindTexture();
  };

  d->emptyTextureID       = bindTexture({  0,   0,   0, 255});
  d->emptyNormalTextureID = bindTexture({127, 127, 255, 255});

  if(map->extendedFBO() == ExtendedFBO::Yes)
    d->compile(vertShaderStr(), fragShaderStr(), ShaderType::RenderExtendedFBO);
  else
    d->compile(vertShaderStr(), fragShaderStr(), ShaderType::Render);

  d->compile(vertShaderStrPicking(), fragShaderStrPicking(), ShaderType::Picking);
}

//##################################################################################################
StaticLightShader::~StaticLightShader()
{
  delete d;
}

//##################################################################################################
void StaticLightShader::use(ShaderType shaderType)
{
  d->shaderType = shaderType;

  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}

//##################################################################################################
void StaticLightShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 mv = v*m;
  glm::mat4 mvp = p*v*m;
  glm::mat4 mInv = glm::inverse(m);

  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniformMatrix4fv(locations.   mMatrixLocation, 1, GL_FALSE, glm::value_ptr(m   ));
    glUniformMatrix4fv(locations.  mvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mv  ));
    glUniformMatrix4fv(locations. mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp ));
    glUniformMatrix4fv(locations.   vMatrixLocation, 1, GL_FALSE, glm::value_ptr(v   ));
    glUniformMatrix4fv(locations.mInvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mInv));

    if(locations.cameraOriginLocation>=0)
    {
      glm::vec3 cameraOrigin_world = glm::inverse(v) * glm::vec4(0,0,0,1);
      glUniform3fv(locations.cameraOriginLocation, 1, &cameraOrigin_world.x);
    }
  };

  switch(d->shaderType)
  {
  case ShaderType::Render:
  {
    exec(d->renderLocations);
    break;
  }

  case ShaderType::RenderExtendedFBO:
  {
    exec(d->renderHDRLocations);
    break;
  }

  case ShaderType::Picking:
  {
    glUniformMatrix4fv(d->pickingMVPMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    break;
  }

  case ShaderType::Light: break;
  }
}

//##################################################################################################
void StaticLightShader::setMaterial(const tp_math_utils::Material& material)
{
  setMaterial(material, material.uvTransformation.uvMatrix());
}

//##################################################################################################
void StaticLightShader::setMaterial(const tp_math_utils::Material& material, const glm::mat3& uvMatrix)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f (locations.    materialUseAmbientLocation, material.useAmbient                );
    glUniform1f (locations.    materialUseDiffuseLocation, material.useDiffuse                );
    glUniform1f (locations.      materialUseNdotLLocation, material.useNdotL                  );
    glUniform1f (locations.materialUseAttenuationLocation, material.useAttenuation            );
    glUniform1f (locations.     materialUseShadowLocation, material.useShadow                 );
    glUniform1f (locations.  materialUseLightMaskLocation, material.useLightMask              );
    glUniform1f (locations. materialUseReflectionLocation, material.useReflection             );

    glUniform1i (locations. materialShadowCatcherLocation, material.rayVisibilityShadowCatcher);

    glUniform1f(locations.    materialAlbedoScaleLocation, material.albedoScale               );
    glUniform1f(locations.    materialAlbedoBrightnessLocation, material.albedoBrightness     );
    glUniform1f(locations.    materialAlbedoContrastLocation  , material.albedoContrast       );
    glUniform1f(locations.    materialAlbedoGammaLocation     , material.albedoGamma          );
    glUniform1f(locations.    materialAlbedoHueLocation       , material.albedoHue            );
    glUniform1f(locations.    materialAlbedoSaturationLocation, material.albedoSaturation     );
    glUniform1f(locations.    materialAlbedoValueLocation     , material.albedoValue          );
    glUniform1f(locations.    materialAlbedoFactorLocation    , material.albedoFactor         );

    glUniformMatrix3fv(locations.uvMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);

  else if(d->shaderType == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void StaticLightShader::invalidate()
{
  d->emptyTextureID = 0;
  d->emptyNormalTextureID = 0;

  Geometry3DShader::invalidate();
}

//##################################################################################################
void StaticLightShader::setTextures(GLuint rgbaTextureID,
                                    GLuint normalsTextureID,
                                    GLuint rmttrTextureID)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbaTextureID);
    glUniform1i(locations.rgbaTextureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalsTextureID);
    glUniform1i(locations.normalsTextureLocation, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rmttrTextureID);
    glUniform1i(locations.rmttrTextureLocation, 2);
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);
  else if(d->shaderType == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void StaticLightShader::setBlankTextures()
{
  setTextures(d->emptyTextureID,
              d->emptyNormalTextureID,
              d->emptyTextureID);
}

//##################################################################################################
void StaticLightShader::setDiscardOpacity(float discardOpacity)
{
  auto exec = [&](const UniformLocations_lt& locations)
  {
    glUniform1f(locations.discardOpacityLocation, discardOpacity);
  };

  if(d->shaderType == ShaderType::Render)
    exec(d->renderLocations);
  else if(d->shaderType == ShaderType::RenderExtendedFBO)
    exec(d->renderHDRLocations);
}

//##################################################################################################
void StaticLightShader::init(RenderInfo& renderInfo,
                             const Matrices& m,
                             const glm::mat4& modelToWorldMatrix)
{
  if(renderInfo.shaderType() == ShaderType::Render ||
     renderInfo.shaderType() == ShaderType::RenderExtendedFBO ||
     renderInfo.shaderType() == ShaderType::Picking)
  {
    use(renderInfo.shaderType());
    setMatrix(modelToWorldMatrix, m.v, m.p);
  }
}

//##################################################################################################
void StaticLightShader::setMaterial(RenderInfo& renderInfo,
                                    const ProcessedGeometry3D& processedGeometry3D)
{
  if(renderInfo.shaderType() == ShaderType::Render)
  {
    const auto& material = processedGeometry3D.alternativeMaterial->material;
    glm::mat3 uvMatrix = processedGeometry3D.uvMatrix * material.uvTransformation.uvMatrix();

    setMaterial(processedGeometry3D.alternativeMaterial->material, uvMatrix);

    setTextures(processedGeometry3D.alternativeMaterial->rgbaTextureID,
                processedGeometry3D.alternativeMaterial->normalsTextureID,
                processedGeometry3D.alternativeMaterial->rmttrTextureID);

    setDiscardOpacity((renderInfo.pass == RenderPass::Transparency)?0.01f:0.80f);
  }
}

//##################################################################################################
void StaticLightShader::setMaterialPicking(RenderInfo& renderInfo,
                                           const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void StaticLightShader::draw(RenderInfo& renderInfo,
                             const ProcessedGeometry3D& processedGeometry3D,
                             GLenum mode,
                             VertexBuffer* vertexBuffer)
{
  TP_UNUSED(processedGeometry3D);
  if(renderInfo.pass == RenderPass::LightFBOs)
    return;

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void StaticLightShader::drawPicking(RenderInfo& renderInfo,
                                    const ProcessedGeometry3D& processedGeometry3D,
                                    GLenum mode,
                                    VertexBuffer* vertexBuffer,
                                    const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);

  if(processedGeometry3D.alternativeMaterial->material.rayVisibilityShadowCatcher)
    return;

  glDisable(GL_BLEND);
  glUniform4fv(d->pickingIDLocation, 1, &pickingID.x);
  d->draw(mode, vertexBuffer);
}

}
