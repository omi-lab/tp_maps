#ifndef tp_maps_PostLayer_h
#define tp_maps_PostLayer_h

#include "tp_maps/Layer.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep

#include "tp_utils/RefCount.h"

namespace tp_maps
{

class PostShader;

//##################################################################################################
class TP_MAPS_EXPORT PostLayer: public Layer
{
  friend class Map;
  TP_REF_COUNT_OBJECTS("PostLayer");
  TP_DQ;
public:
  //################################################################################################
  PostLayer(const RenderPass& customRenderPass);

  //################################################################################################
  ~PostLayer() override;

  //################################################################################################
  void setFrameCoordinateSystem(const tp_utils::StringID& frameCoordinateSystem);

  //################################################################################################
  //! If true just blit read to draw buffers.
  bool bypass() const;

  //################################################################################################
  void setBypass(bool bypass);

  //################################################################################################
  //! Make the post layer a rectangle, size=1=fullscreen
  void setRectangle(const glm::vec2& size);

  //################################################################################################
  //! Make the post layer a rectangle, size=1=fullscreen
  /*!
  Typically this is used where the coordinateSystem() chosen for this layer view a larger area of
  the scene and you want to place a frame around it. Think of the camera view in blender where you
  have the camera in the middle surrounded by a semi transparent frame. This mode is used to draw
  frames like that.

  \param holeSize at the middle of the frame, 1=fullscreen
  \param size at the outside of the frame, 1=fullscreen
   */
  void setFrame(const glm::vec2& holeSize, const glm::vec2& size);  

  //################################################################################################
  void setBlit(bool blitRectangle, bool blitFrame);


protected:  
  //################################################################################################
  //! Called before each frame to allow the post layer to add the render passes it needs.
  virtual void addRenderPasses(std::vector<RenderPass>& renderPasses);

  //################################################################################################
  static tp_utils::StringID findInputFBO(const std::vector<tp_maps::RenderPass>& c);

  //################################################################################################
  static bool containsPass(const std::vector<tp_maps::RenderPass>& renderPasses, tp_maps::RenderPass pass);

  //################################################################################################
  //! If renderPass.postLayer is set this will be called before the pass starts.
  /*!
  Use this to configure OpenGL state before the pass starts.
  */
  virtual void prepareForRenderPass(const RenderPass& renderPass);

  //################################################################################################
  //! If renderPass.postLayer is set this will be called after a pass has completed.
  /*!
  Use this to cleanup OpenGL state after the pass completes.
  */
  virtual void cleanupAfterRenderPass(const RenderPass& renderPass);

  //################################################################################################
  void render(RenderInfo& renderInfo) override;

  //################################################################################################
  void renderWithShader(PostShader* shader, std::function<void()> bindAdditionalTextures = []{});

  //################################################################################################
  void renderToFbo(PostShader* shader, OpenGLFBO& fbo, const GLuint sourceTexture=0);

  //################################################################################################
  void invalidateBuffers() override;

  //################################################################################################
  virtual PostShader* makeShader()=0;
};

}

#endif
