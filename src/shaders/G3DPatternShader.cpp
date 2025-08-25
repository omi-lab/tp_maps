#include "tp_maps/shaders/G3DPatternShader.h"
#include "tp_maps/Map.h"
#include "tp_maps/Geometry3DPool.h"

#include "tp_utils/DebugUtils.h"

#include "glm/gtc/type_ptr.hpp"

namespace tp_maps
{

//##################################################################################################
struct G3DPatternShader::Private
{
  TP_REF_COUNT_OBJECTS("tp_maps::G3DPatternShader::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  GLint mLocation{0};
  GLint mvpLocation{0};

  GLint uvMatrixLocation{0};
  GLint rgbaTextureLocation{0};
  GLint discardOpacityLocation{0};
  GLint patternSelectionLocation{0};

  PatternSelection patternSelection{PatternSelection::Ghosty};

  //################################################################################################
  void draw(GLenum mode, VertexBuffer* vertexBuffer)
  {
#ifdef TP_VERTEX_ARRAYS_SUPPORTED
    tpBindVertexArray(vertexBuffer->vaoID);
    // glDepthFunc(GL_LESS);
    tpDrawElements(mode, vertexBuffer->indexCount, GL_UNSIGNED_INT, nullptr);
    tpBindVertexArray(0);
#else
    vertexBuffer->bindVBO();
    glDrawArrays(mode, 0, vertexBuffer->indexCount);
#endif
  }
};

//##################################################################################################
G3DPatternShader::G3DPatternShader(Map* map, tp_maps::ShaderProfile shaderProfile):
  Geometry3DShader(map, shaderProfile),
  d(new Private())
{

}

//##################################################################################################
G3DPatternShader::~G3DPatternShader()
{
  delete d;
}

//##################################################################################################
void G3DPatternShader::setMatrix(const glm::mat4& m, const glm::mat4& mvp)
{
  glUniformMatrix4fv(d->mLocation, 1, GL_FALSE, glm::value_ptr(m));
  glUniformMatrix4fv(d->mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
}

//##################################################################################################
bool G3DPatternShader::initPass(RenderInfo& renderInfo,
                        const Matrices& m,
                        const glm::mat4& modelToWorldMatrix)
{
  use(renderInfo.shaderType());
  setMatrix(modelToWorldMatrix, m.vp * modelToWorldMatrix);

  return true;
}

//##################################################################################################
void G3DPatternShader::setMaterial(RenderInfo& renderInfo, const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);

  glm::mat3 uvMatrix = processedGeometry3D.uvMatrix * processedGeometry3D.alternativeMaterial->materialUVMatrix;
  glUniformMatrix3fv(d->uvMatrixLocation, 1, GL_FALSE, glm::value_ptr(uvMatrix));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, processedGeometry3D.alternativeMaterial->rgbaTextureID);
  glUniform1i(d->rgbaTextureLocation, 0);

  bool const isTransparent = (renderInfo.pass == RenderPass::Transparency);
  glUniform1f(d->discardOpacityLocation, isTransparent?0.01f:0.80f);

  glUniform1i(d->patternSelectionLocation, GLint(d->patternSelection));
}

//##################################################################################################
void G3DPatternShader::setMaterialPicking(RenderInfo& renderInfo,
                                      const ProcessedGeometry3D& processedGeometry3D)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
}

//##################################################################################################
void G3DPatternShader::draw(RenderInfo& renderInfo,
                        const ProcessedGeometry3D& processedGeometry3D,
                        GLenum mode,
                        VertexBuffer* vertexBuffer)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DPatternShader::drawPicking(RenderInfo& renderInfo,
                               const ProcessedGeometry3D& processedGeometry3D,
                               GLenum mode,
                               VertexBuffer* vertexBuffer,
                               const glm::vec4& pickingID)
{
  TP_UNUSED(renderInfo);
  TP_UNUSED(processedGeometry3D);
  TP_UNUSED(pickingID);

  d->draw(mode, vertexBuffer);
}

//##################################################################################################
void G3DPatternShader::use(ShaderType shaderType)
{
  //https://webglfundamentals.org/webgl/lessons/webgl-and-alpha.html
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, map()->writeAlpha());

  Shader::use(shaderType);
}
//##################################################################################################
const std::string& G3DPatternShader::vertexShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DPatternShader.vert"};
  return s.dataStr(shaderProfile(), shaderType);
}

//##################################################################################################
const std::string& G3DPatternShader::fragmentShaderStr(ShaderType shaderType)
{
  static ShaderResource s{"/tp_maps/G3DPatternShader.frag"};
  return s.dataStr(shaderProfile(), shaderType);
}

//##################################################################################################
void G3DPatternShader::bindLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  glBindAttribLocation(program, 0, "inVertex");
  glBindAttribLocation(program, 2, "inTexture");
}

//##################################################################################################
void G3DPatternShader::getLocations(GLuint program, ShaderType shaderType)
{
  TP_UNUSED(shaderType);

  auto loc = [](auto program, const auto* name)
  {
    GLint const l = glGetUniformLocation(program, name);
    if(l<0) { tpWarning() << "G3DPatternShader location of : "<< name << " results : " << l; }
    return l;
  };
  d->mvpLocation              = loc(program, "mvp");
  d->uvMatrixLocation         = loc(program, "uvMatrix");
  d->rgbaTextureLocation      = loc(program, "rgbaTexture");
  d->discardOpacityLocation   = loc(program, "discardOpacity");
  d->patternSelectionLocation = loc(program, "patternSelection");
}


//################################################################################################
std::vector<std::string> G3DPatternShader::patternSelections()
{
  return { "Ghosty" };
}

//################################################################################################
std::string G3DPatternShader::patternSelectionToString(PatternSelection patternSelection)
{
  switch(patternSelection)
  {
    case G3DPatternShader::PatternSelection::Ghosty : return "Ghosty";
  }
  return "Ghosty";
}

//################################################################################################
G3DPatternShader::PatternSelection G3DPatternShader::patternSelectionFromString(const std::string& patternSelection)
{
  if(patternSelection == "Ghosty") return G3DPatternShader::PatternSelection::Ghosty;

  return G3DPatternShader::PatternSelection::Ghosty;
}

//################################################################################################
void G3DPatternShader::setpatternSelection(PatternSelection patternSelection)
{
  d->patternSelection = patternSelection;
}

//################################################################################################
G3DPatternShader::PatternSelection G3DPatternShader::patternSelection() const
{
  return d->patternSelection;
}

}
