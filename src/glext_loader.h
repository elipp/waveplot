#pragma once

#ifndef APIENTRY
#define APIENTRY
#endif

#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
typedef char GLchar;
typedef unsigned int GLhandleARB; // this is the #else bracnh of "#ifdef __APPLE__"

#include <stddef.h>
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

#define GL_ARRAY_BUFFER                   0x8892
#define GL_ELEMENT_ARRAY_BUFFER           0x8893
#define GL_FRAMEBUFFER                    0x8D40

#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_TEXTURE0                       0x84C0
#define GL_COLOR_ATTACHMENT0              0x8CE0

#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

#define GL_FRAMEBUFFER_COMPLETE           0x8CD5
#define GL_LINK_STATUS                    0x8B82
#define GL_COMPILE_STATUS                 0x8B81
#define GL_INFO_LOG_LENGTH                0x8B84


typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint *params);
extern PFNGLGETSHADERIVPROC glGetShaderiv;

typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;

typedef GLuint(APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
extern PFNGLCREATESHADERPROC glCreateShader;

typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
extern PFNGLATTACHSHADERPROC glAttachShader;

typedef void (APIENTRYP PFNGLCOMPILESHADERARBPROC) (GLhandleARB shaderObj);
extern PFNGLCOMPILESHADERARBPROC glCompileShader;

typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint *params);
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;

typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
extern PFNGLLINKPROGRAMPROC glLinkProgram;

typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
extern PFNGLSHADERSOURCEPROC glShaderSource;

typedef GLuint(APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
extern PFNGLCREATEPROGRAMPROC glCreateProgram;

typedef GLuint(APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
extern PFNGLCREATESHADERPROC glCreateShader;

typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
extern PFNGLBINDBUFFERPROC glBindBuffer;

typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern PFNGLBUFFERDATAPROC glBufferData;

typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;

typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint *buffers);
extern PFNGLGENBUFFERSPROC glGenBuffers;

typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
extern PFNGLACTIVETEXTUREPROC glActiveTexture;

typedef void (APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar *name);
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;

typedef void (APIENTRYP PFNGLBINDFRAGDATALOCATIONPROC) (GLuint program, GLuint color, const GLchar *name);
extern PFNGLBINDFRAGDATALOCATIONPROC glBindFragDataLocation;

typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;

typedef void (APIENTRYP PFNGLCLIENTACTIVETEXTUREPROC) (GLenum texture);
extern PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;

typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint *buffers);
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum *bufs);
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;

typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint index);
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTUREPROC) (GLenum target, GLenum attachment, GLuint texture, GLint level);
extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;

typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint *framebuffers);
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;

typedef GLint(APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar *name);
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

typedef GLenum(APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;

typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
extern PFNGLUSEPROGRAMPROC glUseProgram;

typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

typedef void (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
extern PFNGLUNIFORM1IPROC glUniform1i;

int load_GL_extensions();