#ifndef TEXT_H
#define TEXT_H

#include "gl_includes.h"

#include <string>

#include "definitions.h"
#include "precalculated_texcoords.h"
#include "lin_alg.h"

struct wpstring { 

	std::string text;
	const std::size_t length;	// not really needed
	int x, y;
	bufferObject bufObj;
	wpstring(const std::string &text, const std::size_t& str_len, GLuint x, GLuint y);
	void updateString(const std::string &newtext);
	glyph* generateGlyphs();
	bufferObject generateTextObject();

};

GLuint texcoord_index_from_char(char a);




#endif
