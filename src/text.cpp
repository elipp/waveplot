#include "text.h"

wpstring::wpstring(const std::string& text, const std::size_t& len_max, GLuint x, GLuint y)  : length(len_max) {

	this->x = x;
	this->y = y;
	const std::size_t len = text.length();
	const std::size_t diff = len_max - len;
	std::string newcopy;
	newcopy = text;
	newcopy.append(diff, 0x20);	// this will give the whitespace character when passed though texcoord_index_from_char
	this->text = newcopy;
	bufObj = generateTextObject();
}


void wpstring::updateString(const std::string &newtext) {
	
	const std::size_t new_len = newtext.length();
	if (new_len > length) {	
		const std::string newsub = newtext.substr(0, length);
		this->text = newsub;
	}
	else if (new_len <= length) {
		const std::size_t diff = length - new_len;
		std::string newcopy = newtext;
		newcopy.append(diff, 0x20);
		this->text = newcopy;
	}
	
	glyph *newglyphs = generateGlyphs();	
	glBindBuffer(GL_ARRAY_BUFFER, bufObj.VBOid);
	glBufferData(GL_ARRAY_BUFFER, length*sizeof(glyph), newglyphs, GL_STATIC_DRAW);

	delete [] newglyphs;
}

glyph* wpstring::generateGlyphs() {
	
	glyph *glyphs = new glyph[length];

	float a;
	
	unsigned int i = 0,j = 0;

	for (; i < length; i++) {
		a = i * 7.0;	// the distance between two consecutive letters.
		for (j=0; j < 4; j++) {
			
			glyphs[i].vertices[j].x = x + a +((j>>1)&1)*6.0;
			glyphs[i].vertices[j].y = y + (((j+1)>>1)&1)*11.0;
			glyphs[i].vertices[j].u = glyph_texcoords[texcoord_index_from_char(text[i])][2*j];
			glyphs[i].vertices[j].v = glyph_texcoords[texcoord_index_from_char(text[i])][2*j+1];
		}
	}
	return glyphs;

}

bufferObject wpstring::generateTextObject() {
	
	const std::size_t str_len = text.length();
	glyph *glyphs = generateGlyphs();
	bufferObject textBufObj;
	glGenBuffers(1, &textBufObj.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, textBufObj.VBOid);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glyph) * str_len, (const GLvoid*)glyphs, GL_STATIC_DRAW); 

	delete [] glyphs;

	// generate index buffer object

	glGenBuffers(1, &textBufObj.IBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textBufObj.IBOid);

	unsigned int i = 0;
	unsigned int j = 0;
	
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


