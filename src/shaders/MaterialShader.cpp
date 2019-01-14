#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/Map.h"

#include "tp_math_utils/Globals.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderString vertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//MaterialShader vertexShaderStr\n"
    "\n"
    "$TP_GLSL_IN_V$vec3 inVertex;\n"
    "$TP_GLSL_IN_V$vec3 inNormal;\n"
    "\n"
    "$TP_GLSL_IN_V$vec3 positionWorldspace;\n"
    "\n"
    "$TP_GLSL_OUT_V$vec3 lightVector0;\n"
    "$TP_GLSL_OUT_V$vec3 EyeNormal;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec3 normal;\n"
    "$TP_GLSL_OUT_V$vec3 fragPos;\n"
    "\n"
    "uniform mat4 m;\n"
    "uniform mat4 v;\n"
    "uniform mat4 p;\n"
    "uniform mat4 mvp;\n"
    "uniform mat4 vp;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = mvp * vec4(inVertex, 1.0);\n"
    "  fragPos = mat3(m)*inVertex;\n"
    "  lightVector0 = vec3(0.0, 0.0, 1.0);\n"
    "  normal = mat3(m)*inNormal;\n"
    "}\n";

ShaderString fragmentShaderStr =
    "$TP_FRAG_SHADER_HEADER$"
    "//MaterialShader fragmentShaderStr\n"
    "\n"
    "struct Material\n"
    "{\n"
    "  vec3 ambient;\n"
    "  vec3 diffuse;\n"
    "  vec3 specular;\n"
    "  float shininess;\n"
    "  float alpha;\n"
    "};\n"
    "\n"
    "struct Light\n"
    "{\n"
    "  vec3 position;\n"
    "  vec3 ambient;\n"
    "  vec3 diffuse;\n"
    "  vec3 specular;\n"
    "  float diffuseScale;\n"
    "  float diffuseTranslate;\n"
    "};\n"
    "\n"
    "$TP_GLSL_IN_F$vec3 fragPos;\n"
    "$TP_GLSL_IN_F$vec3 normal;\n"
    "\n"
    "uniform vec3 cameraOriginNear;\n"
    "uniform vec3 cameraOriginFar;\n"
    "uniform Material material;\n"
    "uniform Light light;\n"
    "uniform float picking;\n"
    "uniform vec4 pickingID;\n"
    "\n"
    "$TP_GLSL_IN_F$vec3 lightVector0;\n"
    "$TP_GLSL_IN_F$vec3 EyeNormal;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "\n"
    "void main()\n"
    "{\n"
    "  vec3 norm = normalize(normal);\n"
    "  // Ambient\n"
    "  vec3 ambient = light.ambient * material.ambient;\n"
    "  \n"
    "  // Diffuse\n"
    "  float diff = max((dot(norm, lightVector0)+light.diffuseTranslate)*light.diffuseScale, 0.0);\n"
    "  vec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
    "  \n"
    "  // Specular\n"
    "  vec3 incidenceVector = -lightVector0;\n" //a unit vector
    "  vec3 reflectionVector = reflect(incidenceVector, norm);\n" //also a unit vector
    "  vec3 surfaceToCamera = normalize(cameraOriginNear - cameraOriginFar);\n" //also a unit vector
    "  float cosAngle = max(0.0, dot(surfaceToCamera, reflectionVector));\n"
    "  float specularCoefficient = pow(cosAngle, material.shininess);\n"
    "  vec3 specular = specularCoefficient * material.specular * light.specular;\n"
    "  \n"
    "  vec3 result = ambient + diffuse + specular;\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = vec4(result, material.alpha);\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = (picking*pickingID) + ((1.0-picking)*$TP_GLSL_GLFRAGCOLOR$);"
    "}\n";
}

//##################################################################################################
struct MaterialShader::Private
{
  GLint mMatrixLocation          {0};
  GLint vMatrixLocation          {0};
  GLint pMatrixLocation          {0};
  GLint mvpMatrixLocation        {0};
  GLint vpMatrixLocation         {0};

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

  //################################################################################################
  void draw(GLenum mode, MaterialShader::VertexBuffer* vertexBuffer)
  {
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_INT,
                   nullptr);
    tpBindVertexArray(0);
  }
};

//##################################################################################################
MaterialShader::MaterialShader():
  Geometry3DShader(),
  d(new Private())
{
  compile(vertexShaderStr.data(),
          fragmentShaderStr.data(),
          [](GLuint program)
  {
    glBindAttribLocation(program, 0, "inVertex");
    glBindAttribLocation(program, 1, "inNormal");
  },
  [this](GLuint program)
  {
    d->mMatrixLocation             = glGetUniformLocation(program, "m");
    d->vMatrixLocation             = glGetUniformLocation(program, "v");
    d->pMatrixLocation             = glGetUniformLocation(program, "p");
    d->mvpMatrixLocation           = glGetUniformLocation(program, "mvp");
    d->vpMatrixLocation            = glGetUniformLocation(program, "vp");

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

    const char* shaderName = "MaterialShader";
    if(d->mMatrixLocation  <0)tpWarning() << shaderName << " mMatrixLocation  : " << d->mMatrixLocation  ;
    if(d->vMatrixLocation  <0)tpWarning() << shaderName << " vMatrixLocation  : " << d->vMatrixLocation  ;
    if(d->pMatrixLocation  <0)tpWarning() << shaderName << " pMatrixLocation  : " << d->pMatrixLocation  ;
    if(d->mvpMatrixLocation<0)tpWarning() << shaderName << " mvpMatrixLocation: " << d->mvpMatrixLocation;
    if(d->vpMatrixLocation <0)tpWarning() << shaderName << " vpMatrixLocation : " << d->vpMatrixLocation ;

    if(d->cameraOriginNearLocation <0)tpWarning() << shaderName << " cameraOriginNearLocation  : " << d->cameraOriginNearLocation;
    if(d->cameraOriginFarLocation  <0)tpWarning() << shaderName << " cameraOriginFarLocation   : " << d->cameraOriginFarLocation ;
  });
}

//##################################################################################################
MaterialShader::~MaterialShader()
{
  delete d;
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
  glm::mat4 vp = p*v;
  glUniformMatrix4fv(d->mMatrixLocation  , 1, GL_FALSE, glm::value_ptr(m));
  glUniformMatrix4fv(d->vMatrixLocation  , 1, GL_FALSE, glm::value_ptr(v));
  glUniformMatrix4fv(d->pMatrixLocation  , 1, GL_FALSE, glm::value_ptr(p));
  glUniformMatrix4fv(d->mvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvp));
  glUniformMatrix4fv(d->vpMatrixLocation , 1, GL_FALSE, glm::value_ptr(vp));
}

////##################################################################################################
//MaterialShader::VertexBuffer* MaterialShader::generateVertexBuffer(Map* map,
//                                                                   const std::vector<GLuint>& indexes,
//                                                                   const std::vector<MaterialShader::Vertex>& verts)const
//{
//  VertexBuffer* vertexBuffer = new VertexBuffer(map, this);

//  vertexBuffer->vertexCount = GLuint(verts.size());
//  vertexBuffer->indexCount = GLuint(indexes.size());

//  glGenBuffers(1, &vertexBuffer->iboID);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
//  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size()*sizeof(GLuint), indexes.data(), GL_STATIC_DRAW);
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

//  glGenBuffers(1, &vertexBuffer->vboID);
//  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
//  glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(MaterialShader::Vertex), verts.data(), GL_STATIC_DRAW);
//  glBindBuffer(GL_ARRAY_BUFFER, 0);

//  tpGenVertexArrays(1, &vertexBuffer->vaoID);
//  tpBindVertexArray(vertexBuffer->vaoID);

//  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
//  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MaterialShader::Vertex), (void*)(0));
//  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MaterialShader::Vertex), (void*)(sizeof(float)*3));
//  glEnableVertexAttribArray(0);
//  glEnableVertexAttribArray(1);
//  glDisableVertexAttribArray(3);

//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);

//  tpBindVertexArray(0);

//  return vertexBuffer;
//}

////##################################################################################################
//MaterialShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
//  map(map_),
//  shader(shader_)
//{

//}
////##################################################################################################
//MaterialShader::VertexBuffer::~VertexBuffer()
//{
//  if(!vaoID || !shader.shader())
//    return;

//  map->makeCurrent();
//  tpDeleteVertexArrays(1, &vaoID);
//  glDeleteBuffers(1, &iboID);
//  glDeleteBuffers(1, &vboID);
//}

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

}
