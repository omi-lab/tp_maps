#Add something to the build system to handle OpenGL better than we do.
QT += opengl
LIBS += -lGL
#LIBS += -lGLESv3
DEPENDENCIES += tp_math_utils
DEPENDENCIES += tp_triangulation
INCLUDEPATHS += tp_maps/inc/
LIBRARIES    += tp_maps
DEFINES += GL_SILENCE_DEPRECATION

