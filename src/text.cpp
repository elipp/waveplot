#include "text.h"

GLuint texcoord_index_from_char(char a) {

	return (GLuint)a - 0x20;

}

wpstring::wpstring(std::string& text, GLuint x, GLuint y) {

	bufObj = generateTextObject(text, x, y);
	length = text.length();
}


bufferObject generateTextObject(std::string& text, GLuint x, GLuint y) {

	const std::size_t str_len = text.length();

	glyph *glyphs = new glyph[str_len];

	float a;
	
	unsigned int i = 0,j = 0;

	for (; i < str_len; i++) {
		a = i * 7.0;	// the distance between two consecutive letters.
		for (j=0; j < 4; j++) {
			
			glyphs[i].vertices[j].x = x + a +((j>>1)&1)*6.0;
			glyphs[i].vertices[j].y = y + (((j+1)>>1)&1)*11.0;
			glyphs[i].vertices[j].u = glyph_texcoords[texcoord_index_from_char(text[i])][2*j];
			glyphs[i].vertices[j].v = glyph_texcoords[texcoord_index_from_char(text[i])][2*j+1];
		}
	}
	bufferObject textBufObj;
	glGenBuffers(1, &textBufObj.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, textBufObj.VBOid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glyph) * str_len, (const GLvoid*)glyphs, GL_STATIC_DRAW); 

	delete [] glyphs;

	// generate index buffer object

	glGenBuffers(1, &textBufObj.IBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textBufObj.IBOid);

	i = 0;
	j = 0;
	
	// I really doubt any strings of length 10922 or more will ever be provided, so GLushort it is. 
	GLushort *indices = new GLushort[(6*str_len)];	// three indices per triangle, two triangles per line, BUFSIZE lines

	while (i < 6 * str_len) {

		indices[i] = j;
		indices[i+1] = j+1;
		indices[i+2] = j+3;
		indices[i+3] = j+1;
		indices[i+4] = j+2;
		indices[i+5] = j+3;

		i += 6;
		j += 4;


	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*str_len*sizeof(GLushort), (const GLvoid*)indices, GL_STATIC_DRAW);


	return textBufObj;
}


