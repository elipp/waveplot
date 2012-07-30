#ifndef TEXT_H
#define TEXT_H

#include "gl_includes.h"

#include <string>

#include "definitions.h"
#include "precalculated_texcoords.h"

GLuint texcoord_index_from_char(char a);

bufferObject generateTextObject(std::string& text, GLuint x, GLuint y);


#endif
