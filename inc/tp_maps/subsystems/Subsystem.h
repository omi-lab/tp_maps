#ifndef tp_maps_Subsystem_h
#define tp_maps_Subsystem_h

//#define TP_MAPS_SUBSYSTEM_NONE
#define TP_MAPS_SUBSYSTEM_OPENGL

namespace tp_maps
{

//##################################################################################################
class SubsystemBase
{
public:
  //################################################################################################
  SubsystemBase();

  //################################################################################################
  virtual ~SubsystemBase();


};
}

//#ifdef TP_MAPS_SUBSYSTEM_NONE
//#include "tp_maps/subsystems/open_gl/None.h" // IWYU pragma: keep
//#endif

//#ifdef TP_MAPS_SUBSYSTEM_OPENGL
//#include "tp_maps/subsystems/open_gl/OpenGL.h" // IWYU pragma: keep
//#endif

//#ifdef TP_MAPS_SUBSYSTEM_OPENGL_FIXED
//#include "tp_maps/subsystems/open_gl/OpenGLFixed.h" // IWYU pragma: keep
//#endif

//#ifdef TP_MAPS_SUBSYSTEM_VULKAN
//#include "tp_maps/subsystems/open_gl/Vulkan.h" // IWYU pragma: keep
//#endif

//#ifdef TP_MAPS_SUBSYSTEM_DIRECT3D
//#include "tp_maps/subsystems/open_gl/Direct3D.h" // IWYU pragma: keep
//#endif

#endif
