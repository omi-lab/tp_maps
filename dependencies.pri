DEPENDENCIES += tp_image_utils
DEPENDENCIES += tp_triangulation
DEPENDENCIES += tp_rc
DEPENDENCIES += tp_utils

INCLUDEPATHS += tp_maps/inc/
LIBRARIES    += tp_maps
TP_DEPENDENCIES += tp_maps/dependencies/

TP_STATIC_INIT += tp_maps

DEFINES += GL_SILENCE_DEPRECATION
