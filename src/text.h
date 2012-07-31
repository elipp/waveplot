#ifndef TEXT_H
#define TEXT_H

#include "gl_includes.h"

#include <string>

#include "definitions.h"
#include "precalculated_texcoords.h"
#include "lin_alg.h"

GLuint texcoord_index_from_char(char a);

bufferObject generateTextObject(std::string& text, GLuint x, GLuint y);

namespace Text {
	// glOrtho(0.0, WIN_W, WIN_H, 0.0, 0.0, 1.0);

	// this is very much simplified already.
	static const float p[16] = { 2.0/(WIN_W), 0, 0, 0,
								 0, 2.0/(WIN_H), 0, 0,
								 0, 0, -2.0/(1.0), 0,
								 -1.0, 1.0, 1.0, 1.0 };

	const mat4 projection_matrix(&p[0]);	// since this will never change.
	const mat4 modelview_matrix(MAT_IDENTITY);

}


#endif
