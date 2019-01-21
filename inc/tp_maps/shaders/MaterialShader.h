#ifndef tp_maps_MaterialShader_h
#define tp_maps_MaterialShader_h

#include "tp_maps/shaders/Geometry3DShader.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for drawing textured surfaces.
class TP_MAPS_SHARED_EXPORT MaterialShader: public Geometry3DShader
{
public:
  //################################################################################################
  MaterialShader();

  //################################################################################################
  ~MaterialShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  struct Material
  {
    glm::vec3 ambient{1.0f, 0.0f, 0.0f};
    glm::vec3 diffuse{0.4f, 0.0f, 0.0f};
    glm::vec3 specular{0.1f, 0.1f, 0.1f};
    float shininess{32.0f};
    float alpha{1.0f};
  };

  //################################################################################################
  struct Light
  {
    glm::vec3 position{0.0f, 0.0f, 20.0f};
    glm::vec3 ambient{0.4f, 0.4f, 0.4f};
    glm::vec3 diffuse{0.6f, 0.6f, 0.6f};
    glm::vec3 specular{1.0f, 1.0f, 1.0f};
    float diffuseScale{0.5f};              //! Multiplied with the diffuse lighting calculation.
    float diffuseTranslate{1.0f};          //! Added to the diffuse lighting calculation.
  };

  //################################################################################################
  //! Call this to set the material before drawing the geometry
  void setMaterial(const Material& material);

  //################################################################################################
  //! Call this to set the light before drawing the geometry
  void setLight(const Light& light);

  //################################################################################################
  //! Call this to set the camera origin before drawing the geometry
  void setCameraRay(const glm::vec3& near, const glm::vec3& far);

  //################################################################################################
  //! Call this to set the model, view, and projection matrices before drawing the geometry.
  void setMatrix(const glm::mat4& m, const glm::mat4& v, const glm::mat4& p);

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param vertices The points that make up the line.
  */
  void draw(GLenum mode, VertexBuffer* vertexBuffer);

  //################################################################################################
  //! Call this to draw the image for picking
  /*!
  \param vertices The points that make up the line.
  */
  void drawPicking(GLenum mode,
                   VertexBuffer* vertexBuffer,
                   const glm::vec4& pickingID);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return materialShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
