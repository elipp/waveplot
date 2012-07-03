#ifndef SHADER_H
#define SHADER_H
#ifdef _WIN32

#include <Windows.h>
#include <GL\glew.h>

#elif __linux__

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#endif

#include <fstream>
#include "utils.h"

GLchar* readShaderFromFile(const char* filename);
GLint checkShader(GLuint *shaderId, GLint QUERY);

#endif
