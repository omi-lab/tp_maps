#ifndef tp_maps_FontShader_h
#define tp_maps_FontShader_h

#include "tp_maps/Shader.h"
#include "tp_maps/PreparedString.h"

namespace tp_maps
{

//##################################################################################################
//! A shader for rendering fonts.
class TP_MAPS_EXPORT FontShader: public Shader
{
  TP_DQ;
public:
  //################################################################################################
  static inline const tp_utils::StringID& name(){return fontShaderSID();}

  //################################################################################################
  FontShader(Map* map, tp_maps::ShaderProfile shaderProfile);

  //################################################################################################
  ~FontShader() override;

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
    TP_DQ;
    friend class FontShader;
  public:
    //##############################################################################################
    PreparedString(FontRenderer* fontRenderer,
                   const std::u16string& text,
                   const PreparedStringConfig& config=PreparedStringConfig());

    //##############################################################################################
    ~PreparedString() override;

    //##############################################################################################
    void invalidateBuffers() override;

    //################################################################################################
    void regenerateBuffers() override;
  };

  //################################################################################################
  //! Call this to draw the image
  /*!
  \param preparedString The text to render.
  */
  void drawPreparedString(PreparedString& preparedString);

  //################################################################################################
  //! Prepare OpenGL for rendering
  void use(ShaderType shaderType) override;

protected:
  //################################################################################################
  const char* vertexShaderStr(ShaderType shaderType) override;

  //################################################################################################
  const char* fragmentShaderStr(ShaderType shaderType) override;

  //################################################################################################
  void bindLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void getLocations(GLuint program, ShaderType shaderType) override;

  //################################################################################################
  void init() override;
};

}

#endif
