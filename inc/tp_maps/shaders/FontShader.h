#ifndef tp_maps_FontShader_h
#define tp_maps_FontShader_h

#include "tp_maps/Shader.h"
#include "tp_maps/PreparedString.h"

#include "glm/glm.hpp"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering fonts.
class TP_MAPS_SHARED_EXPORT FontShader: public Shader
{
public:
  //################################################################################################
  FontShader(tp_maps::OpenGLProfile openGLProfile, const char* vertexShader=nullptr, const char* fragmentShader=nullptr);

  //################################################################################################
  ~FontShader() override;

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType = ShaderType::Render) override;

  //################################################################################################
  //! Call this to set the camera matrix before drawing the image
  void setMatrix(const glm::mat4& matrix);  

  //################################################################################################
  void setColor(const glm::vec4& color);

  //################################################################################################
  //! Set the texture that will be draw, this needs to be done each frame before drawing
  void setTexture(GLuint textureID);

  //################################################################################################
  class PreparedString : public tp_maps::PreparedString
  {
  public:
    //##############################################################################################
    PreparedString(const Shader *shader,
                   FontRenderer* fontRenderer,
                   const std::u16string& text,
                   const PreparedStringConfig& config=PreparedStringConfig());

    //##############################################################################################
    ~PreparedString() override;

    //##############################################################################################
    void invalidateBuffers() override;

    //################################################################################################
    void regenerateBuffers() override;

  private:
    struct Private;
    Private* d;
    friend struct Private;
    friend class FontShader;
  };

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param preparedString The text to render.
  */
  void drawPreparedString(PreparedString& preparedString);

  //################################################################################################
  static inline const tp_utils::StringID& name(){return fontShaderSID();}

private:
  struct Private;
  Private* d;
  friend struct Private;
};

}

#endif
