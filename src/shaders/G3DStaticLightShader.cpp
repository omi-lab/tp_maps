#include "tp_maps/shaders/G3DStaticLightShader.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Material.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

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
struct G3DStaticLightShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_math_utils::G3DStaticLightShader::Private");
  TP_NONCOPYABLE(Private);

  Q* q;

  UniformLocations_lt locations;

  //################################################################################################
  Private(G3DStaticLightShader* q_):
    q(q_)
  {

  }
};

//##################################################################################################
G3DStaticLightShader::G3DStaticLightShader(Map* map, tp_maps::OpenGLProfile openGLProfile):
  G3DMaterialShader(map, openGLProfile),
  d(new Private(this))
{

}

//##################################################################################################
G3DStaticLightShader::~G3DStaticLightShader()
{
  delete d;
}

//##################################################################################################
void G3DStaticLightShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  switch(currentShaderType())
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glm::mat4 mv = v*m;
    glm::mat4 mvp = p*v*m;
    glm::mat4 mInv = glm::inverse(m);

    glUniformMatrix4fv(d->locations.   mMatrixLocation, 1, GL_FALSE, glm::value_ptr(m   ));
    glUniformMatrix4fv(d->locations.  mvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mv  ));
    glUniformMatrix4fv(d->locations. mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp ));
    glUniformMatrix4fv(d->locations.   vMatrixLocation, 1, GL_FALSE, glm::value_ptr(v   ));
    glUniformMatrix4fv(d->locations.mInvMatrixLocation, 1, GL_FALSE, glm::value_ptr(mInv));

    if(d->locations.cameraOriginLocation>=0)
    {
      glm::vec3 cameraOrigin_world = glm::inverse(v) * glm::vec4(0,0,0,1);
      glUniform3fv(d->locations.cameraOriginLocation, 1, &cameraOrigin_world.x);
    }
    break;
  }

  default:
  {
    G3DMaterialShader::setMatrix(m, v, p);
    break;
  }
  }
}

//##################################################################################################
void G3DStaticLightShader::setMaterial(const tp_math_utils::Material& material, const glm::mat3& uvMatrix)
{
  switch(currentShaderType())
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glUniform1f (d->locations.    materialUseAmbientLocation, material.useAmbient                );
    glUniform1f (d->locations.    materialUseDiffuseLocation, material.useDiffuse                );
    glUniform1f (d->locations.      materialUseNdotLLocation, material.useNdotL                  );
    glUniform1f (d->locations.materialUseAttenuationLocation, material.useAttenuation            );
    glUniform1f (d->locations.     materialUseShadowLocation, material.useShadow                 );
    glUniform1f (d->locations.  materialUseLightMaskLocation, material.useLightMask              );
    glUniform1f (d->locations. materialUseReflectionLocation, material.useReflection             );

    glUniform1i (d->locations. materialShadowCatcherLocation, material.rayVisibilityShadowCatcher);

    glUniform1f(d->locations.    materialAlbedoScaleLocation, material.albedoScale               );
    glUniform1f(d->locations.    materialAlbedoBrightnessLocation, material.albedoBrightness     );
    glUniform1f(d->locations.    materialAlbedoContrastLocation  , material.albedoContrast       );
    glUniform1f(d->locations.    materialAlbedoGammaLocation     , material.albedoGamma          );
    glUniform1f(d->locations.    materialAlbedoHueLocation       , material.albedoHue            );
    glUniform1f(d->locations.    materialAlbedoSaturationLocation, material.albedoSaturation     );
    glUniform1f(d->locations.    materialAlbedoValueLocation     , material.albedoValue          );
    glUniform1f(d->locations.    materialAlbedoFactorLocation    , material.albedoFactor         );

    glUniformMatrix3fv(d->locations.uvMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));
    break;
  }

  default:
  {
    G3DMaterialShader::setMaterial(material, uvMatrix);
    break;
  }
  }
}

//##################################################################################################
void G3DStaticLightShader::setTextures(GLuint rgbaTextureID,
                                       GLuint normalsTextureID,
                                       GLuint rmttrTextureID)
{  
  switch(currentShaderType())
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgbaTextureID);
    glUniform1i(d->locations.rgbaTextureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalsTextureID);
    glUniform1i(d->locations.normalsTextureLocation, 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, rmttrTextureID);
    glUniform1i(d->locations.rmttrTextureLocation, 2);
    break;
  }

  default:
  {
    G3DMaterialShader::setTextures(rgbaTextureID, normalsTextureID, rmttrTextureID);
    break;
  }
  }
}

//##################################################################################################
void G3DStaticLightShader::setDiscardOpacity(float discardOpacity)
{
  switch(currentShaderType())
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glUniform1f(d->locations.discardOpacityLocation, discardOpacity);
    break;
  }

  default:
  {
    G3DMaterialShader::setDiscardOpacity(discardOpacity);
    break;
  }
  }
}

//##################################################################################################
const char* G3DStaticLightShader::vertexShaderStr(ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    static ShaderResource s{"/tp_maps/G3DStaticLightShader.render.vert"};
    return s.data(openGLProfile(), shaderType);
  }

  case ShaderType::Picking:
  return G3DMaterialShader::vertexShaderStr(shaderType);

  case ShaderType::Light:
  break;
  }

  return nullptr;
}

//##################################################################################################
const char* G3DStaticLightShader::fragmentShaderStr(ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    static ShaderResource s{"/tp_maps/G3DStaticLightShader.render.frag"};
    return s.data(openGLProfile(), shaderType);
  }

  case ShaderType::Picking:
  return G3DMaterialShader::fragmentShaderStr(shaderType);

  case ShaderType::Light:
  break;
  }

  return nullptr;
}

//##################################################################################################
void G3DStaticLightShader::bindLocations(GLuint program, ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
    glBindAttribLocation(program, 2, "inTangent");
    glBindAttribLocation(program, 3, "inTexture");
    break;
  }

  case ShaderType::Picking:
  {
    G3DMaterialShader::bindLocations(program, shaderType);
    break;
  }

  case ShaderType::Light:
  break;
  }
}

//##################################################################################################
void G3DStaticLightShader::getLocations(GLuint program, ShaderType shaderType)
{
  switch(shaderType)
  {
  case ShaderType::Render: [[fallthrough]];
  case ShaderType::RenderExtendedFBO:
  {
    auto loc = [](auto program, const auto* name){return glGetUniformLocation(program, name);};

    d->locations.mMatrixLocation                = loc(program, "m");
    d->locations.mvMatrixLocation               = loc(program, "mv");
    d->locations.mvpMatrixLocation              = loc(program, "mvp");
    d->locations.vMatrixLocation                = loc(program, "v");
    d->locations.mInvMatrixLocation             = loc(program, "mInv");
    d->locations.uvMatrixLocation               = loc(program, "uvMatrix");

    d->locations.cameraOriginLocation           = loc(program, "cameraOrigin_world");

    d->locations.    materialUseAmbientLocation = loc(program, "material.useAmbient"    );
    d->locations.    materialUseDiffuseLocation = loc(program, "material.useDiffuse"    );
    d->locations.      materialUseNdotLLocation = loc(program, "material.useNdotL"      );
    d->locations.materialUseAttenuationLocation = loc(program, "material.useAttenuation");
    d->locations.     materialUseShadowLocation = loc(program, "material.useShadow"     );
    d->locations.  materialUseLightMaskLocation = loc(program, "material.useLightMask"  );
    d->locations. materialUseReflectionLocation = loc(program, "material.useReflection" );

    d->locations.materialShadowCatcherLocation  = loc(program, "material.rayVisibilityShadowCatcher");

    d->locations.  materialAlbedoScaleLocation      = loc(program, "material.albedoScale"  );
    d->locations.  materialAlbedoBrightnessLocation = loc(program, "material.albedoBrightness"  );
    d->locations.  materialAlbedoContrastLocation   = loc(program, "material.albedoContrast"    );
    d->locations.  materialAlbedoGammaLocation      = loc(program, "material.albedoGamma"       );
    d->locations.  materialAlbedoHueLocation        = loc(program, "material.albedoHue"         );
    d->locations.  materialAlbedoSaturationLocation = loc(program, "material.albedoSaturation"  );
    d->locations.  materialAlbedoValueLocation      = loc(program, "material.albedoValue"       );
    d->locations.  materialAlbedoFactorLocation     = loc(program, "material.albedoFactor"      );

    d->locations.txlSizeLocation                = loc(program, "txlSize");
    d->locations.discardOpacityLocation         = loc(program, "discardOpacity");
    d->locations.lightOffsetsLocation           = loc(program, "lightOffsets");

    d->locations.     rgbaTextureLocation       = loc(program, "rgbaTexture"     );
    d->locations.  normalsTextureLocation       = loc(program, "normalsTexture"  );
    d->locations.    rmttrTextureLocation       = loc(program, "rmttrTexture"    );
    break;
  }

  default:
  {
    G3DMaterialShader::getLocations(program, shaderType);
    break;
  }
  }
}

//################################################################################################
void G3DStaticLightShader::init()
{
  if(map()->extendedFBO() == ExtendedFBO::Yes)
    compile(ShaderType::RenderExtendedFBO);
  else
    compile(ShaderType::Render);

#ifdef TP_GLSL_PICKING_SUPPORTED
  compile(ShaderType::Picking);
#endif
}

}
