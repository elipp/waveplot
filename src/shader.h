#ifndef SHADER_H
#define SHADER_H

#include "gl_includes.h"

#include <fstream>
#include "utils.h"

GLchar* readShaderFromFile(const char* filename);
GLint checkShaderCompileStatus(GLuint shaderId);
GLint checkProgramLinkStatus(GLuint programHandle);

#endif
