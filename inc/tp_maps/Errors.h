#ifndef tp_maps_Errors_h
#define tp_maps_Errors_h

#include "tp_maps/Globals.h"
#include "tp_maps/subsystems/open_gl/OpenGL.h"

namespace tp_maps
{
class Map;

//##################################################################################################
class TP_MAPS_EXPORT Errors
{
  TP_NONCOPYABLE(Errors);
  TP_DQ;
public:
  //################################################################################################
  Errors(Map* map);

  //################################################################################################
  ~Errors();

  //################################################################################################
  void initializeGL();

  //################################################################################################
  //! Print OpenGL errors
  /*!
  \param description - This will be printed with the OpenGL error.
  */
  static void printOpenGLError(const std::string& description);

  //################################################################################################
  static void printOpenGLError(const std::string& description, GLenum error);

  //################################################################################################
  //! Prints error and returns true if there is an FBO error detected.
  static bool printFBOError(OpenGLFBO& buffer, const std::string& description);
};

}

#endif
