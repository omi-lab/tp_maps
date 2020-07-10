#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{
ShaderResource vertShaderStr{"/tp_maps/MaterialShader.vert"};
ShaderResource fragShaderStr{"/tp_maps/MaterialShader.frag"};
}

//##################################################################################################
struct MaterialShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::MaterialShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint mMatrixLocation          {0};
  // GLint vMatrixLocation          {0};
  // GLint pMatrixLocation          {0};
  GLint mvMatrixLocation         {0};
  GLint mvpMatrixLocation        {0};
  // GLint vpMatrixLocation         {0};

  GLint cameraOriginNearLocation {0};
  GLint cameraOriginFarLocation  {0};

  GLint materialAmbientLocation  {0};
  GLint materialDiffuseLocation  {0};
  GLint materialSpecularLocation {0};
  GLint materialShininessLocation{0};
  GLint materialAlphaLocation    {0};

  GLint lightPositionLocation        {0};
  GLint lightAmbientLocation         {0};
  GLint lightDiffuseLocation         {0};
  GLint lightSpecularLocation        {0};
  GLint lightDiffuseScaleLocation    {0};
  GLint lightDiffuseTranslateLocation{0};

  GLint pickingLocation          {0};
  GLint pickingIDLocation        {0};

  GLint ambientTextureLocation {0};
  GLint diffuseTextureLocation {0};
  GLint specularTextureLocation{0};
  //GLint alphaTextureIDLocation {0};
  GLint bumpTextureIDLocation  {0};

  //################################################################################################
  void draw(GLenum mode, MaterialShader::VertexBuffer* vertexBuffer)
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
};

//##################################################################################################
MaterialShader::MaterialShader(tp_maps::OpenGLProfile openGLProfile, bool compileShader):
  Geometry3DShader(openGLProfile),
  d(new Private())
{
  if(compileShader)
    compile(vertShaderStr.data(openGLProfile), fragShaderStr.data(openGLProfile), [](auto){}, [](auto){});
}

//##################################################################################################
MaterialShader::~MaterialShader()
{
  delete d;
}

//################################################################################################
void MaterialShader::compile(const char* vertShaderStr,
                             const char* fragShaderStr,
                             const std::function<void(GLuint)>& bindLocations,
                             const std::function<void(GLuint)>& getLocations)
{
  Shader::compile(vertShaderStr,
                  fragShaderStr,
                  [&](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
    glBindAttribLocation(program, 2, "inTangent");
    glBindAttribLocation(program, 3, "inBitangent");
    glBindAttribLocation(program, 4, "inTexture");
    bindLocations(program);
  },
  [&](GLuint program)
  {
    d->mMatrixLocation             = glGetUniformLocation(program, "m");
    // d->vMatrixLocation             = glGetUniformLocation(program, "v");
    // d->pMatrixLocation             = glGetUniformLocation(program, "p");
    d->mvMatrixLocation            = glGetUniformLocation(program, "mv");
    d->mvpMatrixLocation           = glGetUniformLocation(program, "mvp");
    // d->vpMatrixLocation            = glGetUniformLocation(program, "vp");


    d->cameraOriginNearLocation    = glGetUniformLocation(program, "cameraOriginNear");
    d->cameraOriginFarLocation     = glGetUniformLocation(program, "cameraOriginFar");

    d->materialAmbientLocation   = glGetUniformLocation(program, "material.ambient");
    d->materialDiffuseLocation   = glGetUniformLocation(program, "material.diffuse");
    d->materialSpecularLocation  = glGetUniformLocation(program, "material.specular");
    d->materialShininessLocation = glGetUniformLocation(program, "material.shininess");
    d->materialAlphaLocation     = glGetUniformLocation(program, "material.alpha");

    d->lightPositionLocation         = glGetUniformLocation(program, "light.position");
    d->lightAmbientLocation          = glGetUniformLocation(program, "light.ambient");
    d->lightDiffuseLocation          = glGetUniformLocation(program, "light.diffuse");
    d->lightSpecularLocation         = glGetUniformLocation(program, "light.specular");
    d->lightDiffuseScaleLocation     = glGetUniformLocation(program, "light.diffuseScale");
    d->lightDiffuseTranslateLocation = glGetUniformLocation(program, "light.diffuseTranslate");

    d->pickingLocation           = glGetUniformLocation(program, "picking");
    d->pickingIDLocation         = glGetUniformLocation(program, "pickingID");

    d->ambientTextureLocation = glGetUniformLocation(program, "ambientTexture");
    d->diffuseTextureLocation = glGetUniformLocation(program, "diffuseTexture");
    d->specularTextureLocation = glGetUniformLocation(program, "specularTexture");
    //d->alphaTextureIDLocation = glGetUniformLocation(program, "ambientTexture");
    d->bumpTextureIDLocation = glGetUniformLocation(program, "bumpTexture");

    const char* shaderName = "MaterialShader";
    if(d->mMatrixLocation  <0)tpWarning() << shaderName << " mMatrixLocation  : " << d->mMatrixLocation  ;
    // if(d->vMatrixLocation  <0)tpWarning() << shaderName << " vMatrixLocation  : " << d->vMatrixLocation  ;
    // if(d->pMatrixLocation  <0)tpWarning() << shaderName << " pMatrixLocation  : " << d->pMatrixLocation  ;
    if(d->mvMatrixLocation<0)tpWarning() << shaderName << " mvMatrixLocation: " << d->mvMatrixLocation;
    if(d->mvpMatrixLocation<0)tpWarning() << shaderName << " mvpMatrixLocation: " << d->mvpMatrixLocation;
    // if(d->vpMatrixLocation <0)tpWarning() << shaderName << " vpMatrixLocation : " << d->vpMatrixLocation ;

    if(d->cameraOriginNearLocation <0)tpWarning() << shaderName << " cameraOriginNearLocation  : " << d->cameraOriginNearLocation;
    if(d->cameraOriginFarLocation  <0)tpWarning() << shaderName << " cameraOriginFarLocation   : " << d->cameraOriginFarLocation ;
    getLocations(program);
  });
}

//##################################################################################################
void MaterialShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_FALSE);

  Shader::use(shaderType);

  glUniform1f(d->pickingLocation, 0.0f);
  glUniform4f(d->pickingIDLocation, 0.0f, 0.0f, 0.0f, 0.0f);
}

//##################################################################################################
void MaterialShader::setMaterial(const Material& material)
{
  glUniform3fv(d->materialAmbientLocation,  1, &material.ambient.x  );
  glUniform3fv(d->materialDiffuseLocation,  1, &material.diffuse.x  );
  glUniform3fv(d->materialSpecularLocation, 1, &material.specular.x );
  glUniform1f(d->materialShininessLocation,     material.shininess  );
  glUniform1f(d->materialAlphaLocation,         material.alpha      );
}

//##################################################################################################
void MaterialShader::setLight(const Light& light)
{
  glUniform3fv(d->lightAmbientLocation , 1,    &light.ambient.x       );
  glUniform3fv(d->lightDiffuseLocation , 1,    &light.diffuse.x       );
  glUniform3fv(d->lightSpecularLocation, 1,    &light.specular.x      );
  glUniform1f(d->lightDiffuseScaleLocation,     light.diffuseScale    );
  glUniform1f(d->lightDiffuseTranslateLocation, light.diffuseTranslate);
}

//##################################################################################################
void MaterialShader::setCameraRay(const glm::vec3& near, const glm::vec3& far)
{
  glUniform3fv(d->cameraOriginNearLocation, 1, &near.x);
  glUniform3fv(d->cameraOriginFarLocation, 1, &far.x);
}

//##################################################################################################
void MaterialShader::setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p)
{
  glm::mat4 mvp = p*v*m;
  glm::mat3 mv = v*m;
  // glm::mat4 vp = p*v;
  glUniformMatrix4fv(d->mMatrixLocation  , 1, GL_FALSE, glm::value_ptr(m));
  // glUniformMatrix4fv(d->vMatrixLocation  , 1, GL_FALSE, glm::value_ptr(v));
  // glUniformMatrix4fv(d->pMatrixLocation  , 1, GL_FALSE, glm::value_ptr(p));
  glUniformMatrix3fv(d->mvMatrixLocation,  1, GL_FALSE, glm::value_ptr(mv));
  glUniformMatrix4fv(d->mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
  // glUniformMatrix4fv(d->vpMatrixLocation , 1, GL_FALSE, glm::value_ptr(vp));
}

//##################################################################################################
void MaterialShader::draw(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::drawPicking(GLenum mode,
                                 VertexBuffer* vertexBuffer,
                                 const glm::vec4& pickingID)
{
  glDisable(GL_BLEND);
  glUniform1f(d->pickingLocation, 1.0f);
  glUniform4fv(d->pickingIDLocation, 1, &pickingID.x);
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer)
{
  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void MaterialShader::setTextures(GLuint ambientTextureID,
                                 GLuint diffuseTextureID,
                                 GLuint specularTextureID,
                                 GLuint alphaTextureID,
                                 GLuint bumpTextureID)
{
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, ambientTextureID);
  glUniform1i(d->ambientTextureLocation, 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, diffuseTextureID);
  glUniform1i(d->diffuseTextureLocation, 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, specularTextureID);
  glUniform1i(d->specularTextureLocation, 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, alphaTextureID);
  // glUniform1i(d->alphaTextureIDLocation, 3);

  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, bumpTextureID);
  glUniform1i(d->bumpTextureIDLocation, 4);
}

}
