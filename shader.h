#ifndef SHADER_H
#define SHADER_H
#include <Windows.h>
#include <GL\glew.h>

#include <fstream>
#include "utils.h"

GLchar* readShaderFromFile(const char* filename);
GLint checkShader(GLuint *shaderId, GLint QUERY);

#endif
