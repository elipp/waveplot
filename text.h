#ifndef TEXT_H
#define TEXT_H
#ifdef _WIN32
#include <GL/glew.h>

#elif __linux__
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#endif

#include <string>

#include "definitions.h"
#include "precalculated_texcoords.h"

GLuint texcoord_index_from_char(char a);

bufferObject generateTextObject(std::string& text, GLuint x, GLuint y);


#endif
