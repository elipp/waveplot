#include <Windows.h>
#include <gl/gl.h>
#include <wingdi.h>
#include <cassert>

#include "glext_loader.h"

PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLCOMPILESHADERARBPROC glCompileShader;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLBUFFERSUBDATAPROC glBufferSubData;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
PFNGLDRAWBUFFERSPROC glDrawBuffers;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGENERATEMIPMAPPROC glGenerateMipmap;

int load_GL_extensions() {

	glGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	assert(glGetShaderiv);
	
	glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");
	assert(glGetShaderInfoLog);

	glCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	assert(glCreateShader);

	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	assert(glAttachShader);

	glCompileShader = (PFNGLCOMPILESHADERARBPROC)wglGetProcAddress("glCompileShader");
	assert(glCompileShader);

	glGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	assert(glGetProgramiv);

	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	assert(glLinkProgram);

	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	assert(glShaderSource);
	
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	assert(glCreateProgram);

	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	assert(glBindBuffer);

	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	assert(glBufferData);

	glBufferSubData = (PFNGLBUFFERSUBDATAPROC)wglGetProcAddress("glBufferSubData");
	assert(glBufferSubData);
	
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	assert(glGenBuffers);
	
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	assert(glActiveTexture);

	glBindAttribLocation = (PFNGLBINDATTRIBLOCATIONPROC)wglGetProcAddress("glBindAttribLocation");
	assert(glBindAttribLocation);

	glBindFragDataLocation = (PFNGLBINDFRAGDATALOCATIONPROC)wglGetProcAddress("glBindFragDataLocation");
	assert(glBindFragDataLocation);

	glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
	assert(glBindFramebuffer);

	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
	assert(glClientActiveTexture);

	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
	assert(glDeleteBuffers);

	glDrawBuffers = (PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers");
	assert(glDrawBuffers);

	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)wglGetProcAddress("glEnableVertexAttribArray");
	assert(glEnableVertexAttribArray);

	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glFramebufferTexture");
	assert(glFramebufferTexture);

	glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
	assert(glGenFramebuffers);

	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	assert(glGetUniformLocation);

	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
	assert(glUniformMatrix4fv);

	glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
	assert(glCheckFramebufferStatus);

	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	assert(glUseProgram);

	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)wglGetProcAddress("glVertexAttribPointer");
	assert(glVertexAttribPointer);

	glUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	assert(glUniform1i);

	glGenerateMipmap = (PFNGLGENERATEMIPMAPPROC)wglGetProcAddress("glGenerateMipmap");
	assert(glGenerateMipmap);

	return 1;
}