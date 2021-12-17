TARGET = tp_maps
TEMPLATE = lib

TP_RC += src/tp_maps.qrc

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

SOURCES += src/TexturePool.cpp
HEADERS += inc/tp_maps/TexturePool.h

SOURCES += src/TexturePoolKey.cpp
HEADERS += inc/tp_maps/TexturePoolKey.h

SOURCES += src/Font.cpp
HEADERS += inc/tp_maps/Font.h

SOURCES += src/FontRenderer.cpp
HEADERS += inc/tp_maps/FontRenderer.h

SOURCES += src/PreparedString.cpp
HEADERS += inc/tp_maps/PreparedString.h

SOURCES += src/Geometry3DPool.cpp
HEADERS += inc/tp_maps/Geometry3DPool.h


#-- Picking Results --------------------------------------------------------------------------------
SOURCES += src/picking_results/ImagePickingResult.cpp
HEADERS += inc/tp_maps/picking_results/ImagePickingResult.h

SOURCES += src/picking_results/HandlePickingResult.cpp
HEADERS += inc/tp_maps/picking_results/HandlePickingResult.h

SOURCES += src/picking_results/GeometryPickingResult.cpp
HEADERS += inc/tp_maps/picking_results/GeometryPickingResult.h

SOURCES += src/picking_results/PointsPickingResult.cpp
HEADERS += inc/tp_maps/picking_results/PointsPickingResult.h

SOURCES += src/picking_results/LinesPickingResult.cpp
HEADERS += inc/tp_maps/picking_results/LinesPickingResult.h


#-- Controllers ------------------------------------------------------------------------------------
SOURCES += src/controllers/FlatController.cpp
HEADERS += inc/tp_maps/controllers/FlatController.h

SOURCES += src/controllers/AnimatedFlatController.cpp
HEADERS += inc/tp_maps/controllers/AnimatedFlatController.h

SOURCES += src/controllers/FPSController.cpp
HEADERS += inc/tp_maps/controllers/FPSController.h

SOURCES += src/controllers/FixedController.cpp
HEADERS += inc/tp_maps/controllers/FixedController.h

SOURCES += src/controllers/GraphController.cpp
HEADERS += inc/tp_maps/controllers/GraphController.h

SOURCES += src/controllers/CADController.cpp
HEADERS += inc/tp_maps/controllers/CADController.h


#-- Shaders ----------------------------------------------------------------------------------------
SOURCES += src/shaders/LineShader.cpp
HEADERS += inc/tp_maps/shaders/LineShader.h

SOURCES += src/shaders/PointSpriteShader.cpp
HEADERS += inc/tp_maps/shaders/PointSpriteShader.h

SOURCES += src/shaders/Geometry3DShader.cpp
HEADERS += inc/tp_maps/shaders/Geometry3DShader.h

SOURCES += src/shaders/ImageShader.cpp
HEADERS += inc/tp_maps/shaders/ImageShader.h

SOURCES += src/shaders/YUVImageShader.cpp
HEADERS += inc/tp_maps/shaders/YUVImageShader.h

SOURCES += src/shaders/DepthImageShader.cpp
HEADERS += inc/tp_maps/shaders/DepthImageShader.h

SOURCES += src/shaders/MaterialShader.cpp
HEADERS += inc/tp_maps/shaders/MaterialShader.h

SOURCES += src/shaders/FontShader.cpp
HEADERS += inc/tp_maps/shaders/FontShader.h

SOURCES += src/shaders/FrameShader.cpp
HEADERS += inc/tp_maps/shaders/FrameShader.h

SOURCES += src/shaders/FullScreenShader.cpp
HEADERS += inc/tp_maps/shaders/FullScreenShader.h

SOURCES += src/shaders/BackgroundShader.cpp
HEADERS += inc/tp_maps/shaders/BackgroundShader.h

SOURCES += src/shaders/PatternShader.cpp
HEADERS += inc/tp_maps/shaders/PatternShader.h

SOURCES += src/shaders/PostShader.cpp
HEADERS += inc/tp_maps/shaders/PostShader.h

SOURCES += src/shaders/PostSSAOShader.cpp
HEADERS += inc/tp_maps/shaders/PostSSAOShader.h

SOURCES += src/shaders/PostSSRShader.cpp
HEADERS += inc/tp_maps/shaders/PostSSRShader.h

SOURCES += src/shaders/PostGammaShader.cpp
HEADERS += inc/tp_maps/shaders/PostGammaShader.h

SOURCES += src/shaders/PostBlitShader.cpp
HEADERS += inc/tp_maps/shaders/PostBlitShader.h

SOURCES += src/shaders/PostOutlineShader.cpp
HEADERS += inc/tp_maps/shaders/PostOutlineShader.h

SOURCES += src/shaders/PostBlurAndTintShader.cpp
HEADERS += inc/tp_maps/shaders/PostBlurAndTintShader.h

SOURCES += src/shaders/PostGrid2DShader.cpp
HEADERS += inc/tp_maps/shaders/PostGrid2DShader.h


#-- Layers -----------------------------------------------------------------------------------------
SOURCES += src/layers/GridLayer.cpp
HEADERS += inc/tp_maps/layers/GridLayer.h

SOURCES += src/layers/RulerLayer.cpp
HEADERS += inc/tp_maps/layers/RulerLayer.h

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

SOURCES += src/layers/FBOLayer.cpp
HEADERS += inc/tp_maps/layers/FBOLayer.h

SOURCES += src/layers/LightsLayer.cpp
HEADERS += inc/tp_maps/layers/LightsLayer.h

SOURCES += src/layers/GizmoLayer.cpp
HEADERS += inc/tp_maps/layers/GizmoLayer.h

SOURCES += src/layers/BackgroundLayer.cpp
HEADERS += inc/tp_maps/layers/BackgroundLayer.h


#-- Post Processing Layers -------------------------------------------------------------------------

SOURCES += src/layers/PostLayer.cpp
HEADERS += inc/tp_maps/layers/PostLayer.h

SOURCES += src/layers/PostBlitLayer.cpp
HEADERS += inc/tp_maps/layers/PostBlitLayer.h

SOURCES += src/layers/PostGammaLayer.cpp
HEADERS += inc/tp_maps/layers/PostGammaLayer.h

SOURCES += src/layers/PostSSAOLayer.cpp
HEADERS += inc/tp_maps/layers/PostSSAOLayer.h

SOURCES += src/layers/PostSSRLayer.cpp
HEADERS += inc/tp_maps/layers/PostSSRLayer.h

SOURCES += src/layers/PostOutlineLayer.cpp
HEADERS += inc/tp_maps/layers/PostOutlineLayer.h

SOURCES += src/layers/PostBlurAndTintLayer.cpp
HEADERS += inc/tp_maps/layers/PostBlurAndTintLayer.h

SOURCES += src/layers/PostGrid2DLayer.cpp
HEADERS += inc/tp_maps/layers/PostGrid2DLayer.h


#-- Textures ---------------------------------------------------------------------------------------
SOURCES += src/textures/BasicTexture.cpp
HEADERS += inc/tp_maps/textures/BasicTexture.h

SOURCES += src/textures/DefaultSpritesTexture.cpp
HEADERS += inc/tp_maps/textures/DefaultSpritesTexture.h
