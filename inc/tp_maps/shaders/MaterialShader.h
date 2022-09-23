#ifndef tp_maps_MaterialShader_h
#define tp_maps_MaterialShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

#include "tp_math_utils/Light.h"

namespace tp_math_utils
{
struct Material;
}

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing textured surfaces.
class TP_MAPS_SHARED_EXPORT MaterialShader: public Geometry3DShader
{
public:
  //################################################################################################
  MaterialShader(Map* map, tp_maps::OpenGLProfile openGLProfile, bool compileShader=true);

  //################################################################################################
  ~MaterialShader() override;

  //################################################################################################
  void compile(const char* vertexShaderStr,
               const char* fragmentShaderStr,
               const std::function<void(GLuint)>& bindLocations,
               const std::function<void(GLuint)>& getLocations,
               ShaderType shaderType = ShaderType::Render);

  //################################################################################################
  //! Builds the normal render shader and calls modifyShaders to allow you to alter it.
  void compileRenderShader(const std::function<void(std::string& vert, std::string& frag)>& modifyShaders,
                           const std::function<void(GLuint)>& bindLocations,
                           const std::function<void(GLuint)>& getLocations,
                           ShaderType shaderType = ShaderType::Render);

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the material before drawing the geometry
  void setMaterial(const tp_math_utils::Material& material);

  //################################################################################################
  //! Call this to set the lights before drawing the geometry
  void setLights(const std::vector<tp_math_utils::Light>& lights, const std::vector<FBO>& lightBuffers);

  //################################################################################################
  //! Call this to set the light offsets before drawing the geometry
  void setLightOffsets(size_t renderedlightLevels);

  //################################################################################################
  //! Call this to set the model, view, and projection matrices before drawing the geometry.
  void setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  /*!
  \param rgbaTextureID albedo and alpha texture.
  \param normalsTextureID normals texture.
  \param rmttrTextureID roughness, metalness, transmission, transmission roughness texture.
  */
  void setTextures(GLuint rgbaTextureID,
                   GLuint normalsTextureID,
                   GLuint rmttrTextureID);

  //################################################################################################
  void setBlankTextures();

  //################################################################################################
  //! Discard alpha values less than this
  /*!
  This can be used to discard alpha values on the normal render but draw them on the transparency
  pass. Typically 0.99 would be used for render and 0.01 would be used for transparency passes.
  \param discardOpacity A value between 1 and 0.
  */
  void setDiscardOpacity(float discardOpacity);

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param vertices The points that make up the line.
  */
  virtual void draw(GLenum mode, VertexBuffer* vertexBuffer);

  //################################################################################################
  //! Call this to draw the image for picking
  /*!
  \param vertices The points that make up the line.
  */
  virtual void drawPicking(GLenum mode,
                           VertexBuffer* vertexBuffer,
                           const glm::vec4& pickingID);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return materialShaderSID();}

protected:  
  //################################################################################################
  void invalidate() override;

  //################################################################################################
  void drawVertexBuffer(GLenum mode, VertexBuffer* vertexBuffer);

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
