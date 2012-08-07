#ifndef SHADER_H
#define SHADER_H

#include "gl_includes.h"

#include <fstream>
#include <sstream>
#include "utils.h"

namespace Shader {
	
	static GLint checkShaderCompileStatus(GLuint shaderId);
	static const std::string logfilename("shader.log");

};

class ShaderProgram { 

	GLuint programHandle_;
	bool error;

public:
	
	GLint checkLinkStatus();
	ShaderProgram(const std::string& vs_filename, const std::string& fs_filename, const std::string &gs_filename);

	bool valid() const;

	GLuint programHandle() const { return programHandle_; }
};

#endif
