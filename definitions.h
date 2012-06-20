#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <GL/glu.h>
#include <cstring>
#include <string>


struct WAVHEADERINFO {

        char padding[4];        // 4
        unsigned int chunksize; // 8
        char padding2[14];      // 22
        short int numChannels;  // 24
        int sampleRate;         // 28
        char padding3[6];       // 34
        short int bitDepth;     // 36
        char padding4[8];       // 44

};

struct BMPHEADERINFO {

        // the first two bytes of a bmp file contain the letters "B" and "M".
        // Adding a "signature" element of size 2 to the beginning of this structure
        // would force us to use a struct packing pragma directive
        unsigned int filesize;
        unsigned int dummy0;            // two reserved 2-byte fields, must be zero
        unsigned int dataoffset;
        unsigned int headersize;
        unsigned int width;
        unsigned int height;
        unsigned short dummy1;
        unsigned short bpp;
        unsigned int compression;
        unsigned int size;
        int hres;
        int vres;
        unsigned int colors;
        unsigned int impcolors;

        BMPHEADERINFO() {
                memset(this, 0, sizeof(BMPHEADERINFO));
        }

};

struct texture {

	std::string name;
	GLuint textureId;
	unsigned short width;
	unsigned short height;
	bool hasAlpha;

	bool make_texture(const char* filename, GLint filter_flag);
};



struct vertex {

	float x,y;
	float u,v;
	vertex(float x_, float y_, float u_, float v_) : x(x_), y(y_), u(u_), v(v_) {}
	vertex() {} 

};

struct line {

	vertex vertices[4];

};

typedef line glyph;

struct bufferObject {

	GLuint VBOid;
	GLuint IBOid;

};

struct wpstring { 

	std::size_t length;
	bufferObject bufObj;
	wpstring(std::string &text, GLuint x, GLuint y);

};


#endif
