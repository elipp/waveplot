#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#ifdef _WIN32
#include "windows_gl_includes.h"
#elif __linux__
#include "linux_gl_includes.h"
#endif


#include <cstring>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265
#endif

static const unsigned int BUFSIZE = 10000;	// NUMBER OF SAMPLES, THE ACTUAL BUFFER SIZE
						// WOULD BE BUFSIZE*sample_bit_depth


#ifdef _WIN32

static const unsigned int WIN_W = 1280; 
static const unsigned int WIN_H = 960;

#elif __linux__

static const unsigned int WIN_W = 800; 
static const unsigned int WIN_H = 600;

#endif

static const float aspect_ratio = WIN_W/WIN_H;


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

struct vertex {		// user defined constructors -> not aggregate

	float x,y;
	float u,v;
	vertex(float _x, float _y, float _u, float _v) : x(_x), y(_y), u(_u), v(_v) {}
	vertex() {} 

};

struct line {	// (almost) deprecated xdd

	vertex vertices[4];

};

typedef line glyph;

struct triangle {

	vertex v1;
	vertex v2;
	vertex v3;

};


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
