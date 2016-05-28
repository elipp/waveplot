#ifndef GL_INCLUDES_H
#define	GL_INCLUDES_H

#ifdef _WIN32
#include <Windows.h>

#include <GL\GL.h>	// probably needless to include these after glew.h
#include <GL\GLU.h>

#include "glext_loader.h"

#elif __linux__
#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#endif

#endif
