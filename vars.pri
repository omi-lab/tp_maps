TARGET = tp_maps
TEMPLATE = lib

DEFINES += TP_MAPS_LIBRARY

SOURCES += src/Globals.cpp
HEADERS += inc/tp_maps/Globals.h

SOURCES += src/Map.cpp
HEADERS += inc/tp_maps/Map.h

SOURCES += src/RenderInfo.cpp
HEADERS += inc/tp_maps/RenderInfo.h

SOURCES += src/SpriteTexture.cpp
HEADERS += inc/tp_maps/SpriteTexture.h

#SOURCES += src/MouseEvent.cpp
HEADERS += inc/tp_maps/MouseEvent.h

#SOURCES += src/KeyEvent.cpp
HEADERS += inc/tp_maps/KeyEvent.h

SOURCES += src/PickingResult.cpp
HEADERS += inc/tp_maps/PickingResult.h

SOURCES += src/Controller.cpp
HEADERS += inc/tp_maps/Controller.h

SOURCES += src/Shader.cpp
HEADERS += inc/tp_maps/Shader.h

SOURCES += src/Layer.cpp
HEADERS += inc/tp_maps/Layer.h

SOURCES += src/Texture.cpp
HEADERS += inc/tp_maps/Texture.h

SOURCES += src/Font.cpp
HEADERS += inc/tp_maps/Font.h

SOURCES += src/FontRenderer.cpp
HEADERS += inc/tp_maps/FontRenderer.h

SOURCES += src/PreparedString.cpp
HEADERS += inc/tp_maps/PreparedString.h


#-- Picking Results --------------------------------------------------------------------------------
SOURCES += src/picking_results/ImagePickingResult.cpp
HEADERS += inc/tp_maps/picking_results/ImagePickingResult.h

SOURCES += src/picking_results/HandlePickingResult.cpp
HEADERS += inc/tp_maps/picking_results/HandlePickingResult.h

SOURCES += src/picking_results/GeometryPickingResult.cpp
HEADERS += inc/tp_maps/picking_results/GeometryPickingResult.h

SOURCES += src/picking_results/PointsPickingResult.cpp
HEADERS += inc/tp_maps/picking_results/PointsPickingResult.h


#-- Controllers ------------------------------------------------------------------------------------
SOURCES += src/controllers/FlatController.cpp
HEADERS += inc/tp_maps/controllers/FlatController.h

SOURCES += src/controllers/AnimatedFlatController.cpp
HEADERS += inc/tp_maps/controllers/AnimatedFlatController.h

SOURCES += src/controllers/FPSController.cpp
HEADERS += inc/tp_maps/controllers/FPSController.h


#-- Shaders ----------------------------------------------------------------------------------------
SOURCES += src/shaders/LineShader.cpp
HEADERS += inc/tp_maps/shaders/LineShader.h

SOURCES += src/shaders/ImageShader.cpp
HEADERS += inc/tp_maps/shaders/ImageShader.h

SOURCES += src/shaders/YUVImageShader.cpp
HEADERS += inc/tp_maps/shaders/YUVImageShader.h

SOURCES += src/shaders/PointSpriteShader.cpp
HEADERS += inc/tp_maps/shaders/PointSpriteShader.h

SOURCES += src/shaders/MaterialShader.cpp
HEADERS += inc/tp_maps/shaders/MaterialShader.h

SOURCES += src/shaders/FontShader.cpp
HEADERS += inc/tp_maps/shaders/FontShader.h

SOURCES += src/shaders/FrameShader.cpp
HEADERS += inc/tp_maps/shaders/FrameShader.h


#-- Layers -----------------------------------------------------------------------------------------
SOURCES += src/layers/GridLayer.cpp
HEADERS += inc/tp_maps/layers/GridLayer.h

SOURCES += src/layers/ImageLayer.cpp
HEADERS += inc/tp_maps/layers/ImageLayer.h

SOURCES += src/layers/HandleLayer.cpp
HEADERS += inc/tp_maps/layers/HandleLayer.h

SOURCES += src/layers/GeometryLayer.cpp
HEADERS += inc/tp_maps/layers/GeometryLayer.h

SOURCES += src/layers/Geometry3DLayer.cpp
HEADERS += inc/tp_maps/layers/Geometry3DLayer.h

SOURCES += src/layers/LinesLayer.cpp
HEADERS += inc/tp_maps/layers/LinesLayer.h

SOURCES += src/layers/PointsLayer.cpp
HEADERS += inc/tp_maps/layers/PointsLayer.h

SOURCES += src/layers/FrustumLayer.cpp
HEADERS += inc/tp_maps/layers/FrustumLayer.h

SOURCES += src/layers/PlaneLayer.cpp
HEADERS += inc/tp_maps/layers/PlaneLayer.h


#-- Textures ---------------------------------------------------------------------------------------
SOURCES += src/textures/BasicTexture.cpp
HEADERS += inc/tp_maps/textures/BasicTexture.h

SOURCES += src/textures/DefaultSpritesTexture.cpp
HEADERS += inc/tp_maps/textures/DefaultSpritesTexture.h
