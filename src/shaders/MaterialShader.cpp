#include "tp_maps/shaders/MaterialShader.h"
#include "tp_maps/Map.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

namespace
{

ShaderString vertexShaderStr =
    "$TP_VERT_SHADER_HEADER$"
    "//MaterialShader vertexShaderStr\n"
    "$TP_GLSL_IN_V$vec3 inVertex;\n"
    "$TP_GLSL_IN_V$vec3 inNormal;\n"
    "$TP_GLSL_OUT_V$vec3 LightVector0;\n"
    "$TP_GLSL_OUT_V$vec3 EyeNormal;\n"
    "$TP_GLSL_OUT_V$vec2 texCoordinate;\n"
    "$TP_GLSL_OUT_V$vec3 normal;\n"
    "$TP_GLSL_OUT_V$vec3 fragPos;\n"
    "uniform mat4 matrix;\n"
    "void main()\n"
    "{\n"
    "  gl_Position = matrix * vec4(inVertex, 1.0);\n"
    "  fragPos = inVertex;\n"
    "  LightVector0 = vec3(1.0, 1.0, 1.0);\n"
    "  normal = inNormal;\n"
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
    "};\n"
    "\n"
    "varying vec3 fragPos;\n"
    "varying vec3 normal;\n"
    "\n"
    "uniform vec3 viewPos;\n"
    "uniform Material material;\n"
    "uniform Light light;\n"
    "uniform float picking;\n"
    "uniform vec4 pickingID;\n"
    "\n"
    "varying vec3 LightVector0;\n"
    "varying vec3 EyeNormal;\n"
    "$TP_GLSL_GLFRAGCOLOR_DEF$"
    "\n"
    "void main()\n"
    "{\n"
    "  // Ambient\n"
    "  vec3 ambient = light.ambient * material.ambient;\n"
    "  \n"
    "  // Diffuse\n"
    "  vec3 norm = normalize(normal);\n"
    "  vec3 lightDir = normalize(light.position - fragPos);\n"
    "  float diff = max(dot(norm, lightDir), 0.0);\n"
    "  vec3 diffuse = light.diffuse * (diff * material.diffuse);\n"
    "  \n"
    "  // Specular\n"
    "  vec3 viewDir = normalize(viewPos - fragPos);\n"
    "  vec3 reflectDir = reflect(-lightDir, norm);\n"
    "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);\n"
    "  vec3 specular = light.specular * (spec * material.specular);\n"
    "  \n"
    "  vec3 result = ambient + diffuse + specular;\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = vec4(result, material.alpha);\n"
    "  $TP_GLSL_GLFRAGCOLOR$ = (picking*pickingID) + ((1.0-picking)*$TP_GLSL_GLFRAGCOLOR$);"
    "}\n";
}

//##################################################################################################
struct MaterialShader::Private
{
  GLint matrixLocation           {0};

  GLint materialAmbientLocation  {0};
  GLint materialDiffuseLocation  {0};
  GLint materialSpecularLocation {0};
  GLint materialShininessLocation{0};
  GLint materialAlphaLocation    {0};

  GLint lightPositionLocation    {0};
  GLint lightAmbientLocation     {0};
  GLint lightDiffuseLocation     {0};
  GLint lightSpecularLocation    {0};

  GLint pickingLocation          {0};
  GLint pickingIDLocation        {0};

  //################################################################################################
  void draw(GLenum mode, MaterialShader::VertexBuffer* vertexBuffer)
  {
    tpBindVertexArray(vertexBuffer->vaoID);
    tpDrawElements(mode,
                   vertexBuffer->indexCount,
                   GL_UNSIGNED_SHORT,
                   nullptr);
    tpBindVertexArray(0);
  }
};

//##################################################################################################
MaterialShader::MaterialShader():
  Shader(),
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
    d->matrixLocation            = glGetUniformLocation(program, "matrix");

    d->materialAmbientLocation   = glGetUniformLocation(program, "material.ambient");
    d->materialDiffuseLocation   = glGetUniformLocation(program, "material.diffuse");
    d->materialSpecularLocation  = glGetUniformLocation(program, "material.specular");
    d->materialShininessLocation = glGetUniformLocation(program, "material.shininess");
    d->materialAlphaLocation     = glGetUniformLocation(program, "material.alpha");

    d->lightPositionLocation     = glGetUniformLocation(program, "light.position");
    d->lightAmbientLocation      = glGetUniformLocation(program, "light.ambient");
    d->lightDiffuseLocation      = glGetUniformLocation(program, "light.diffuse");
    d->lightSpecularLocation     = glGetUniformLocation(program, "light.specular");

    d->pickingLocation           = glGetUniformLocation(program, "picking");
    d->pickingIDLocation         = glGetUniformLocation(program, "pickingID");

    const char* shaderName = "MaterialShader";
    if(d->matrixLocation<0)tpWarning() << shaderName << " d->matrixLocation: " << d->matrixLocation;
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
  glUniform3fv(d->materialAmbientLocation,  1, (GLfloat*)&material.ambient  );
  glUniform3fv(d->materialDiffuseLocation,  1, (GLfloat*)&material.diffuse  );
  glUniform3fv(d->materialSpecularLocation, 1, (GLfloat*)&material.specular );
  glUniform1f(d->materialShininessLocation,               material.shininess);
  glUniform1f(d->materialAlphaLocation,                   material.alpha    );
}

//##################################################################################################
void MaterialShader::setLight(const Light& light)
{
  glUniform3fv(d->lightAmbientLocation , 1, (GLfloat*)&light.ambient );
  glUniform3fv(d->lightDiffuseLocation , 1, (GLfloat*)&light.diffuse );
  glUniform3fv(d->lightSpecularLocation, 1, (GLfloat*)&light.specular);
}

//##################################################################################################
void MaterialShader::setMatrix(const glm::mat4& matrix)
{
  glUniformMatrix4fv(d->matrixLocation, 1, GL_FALSE, glm::value_ptr(matrix));
}

//##################################################################################################
MaterialShader::VertexBuffer* MaterialShader::generateVertexBuffer(Map* map,
                                                                   const std::vector<GLushort>& indexes,
                                                                   const std::vector<MaterialShader::Vertex>& verts)const
{
  VertexBuffer* vertexBuffer = new VertexBuffer(map, this);

  vertexBuffer->vertexCount = verts.size();
  vertexBuffer->indexCount = indexes.size();

  glGenBuffers(1, &vertexBuffer->iboID);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size()*sizeof(GLushort), indexes.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glGenBuffers(1, &vertexBuffer->vboID);
  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(MaterialShader::Vertex), verts.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  tpGenVertexArrays(1, &vertexBuffer->vaoID);
  tpBindVertexArray(vertexBuffer->vaoID);

  glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->vboID);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(MaterialShader::Vertex), (void*)(0));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(MaterialShader::Vertex), (void*)(sizeof(float)*3));
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glDisableVertexAttribArray(3);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexBuffer->iboID);

  tpBindVertexArray(0);

  return vertexBuffer;
}

//##################################################################################################
MaterialShader::VertexBuffer::VertexBuffer(Map* map_, const Shader *shader_):
  map(map_),
  shader(shader_)
{

}
//##################################################################################################
MaterialShader::VertexBuffer::~VertexBuffer()
{
  if(!vaoID || !shader.shader())
    return;

  map->makeCurrent();
  tpDeleteVertexArrays(1, &vaoID);
  glDeleteBuffers(1, &iboID);
  glDeleteBuffers(1, &vboID);
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

}
