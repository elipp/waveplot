#include "shader.h"

char* readShaderFromFile(const char* filename)
{
	std::ifstream in(filename, std::ios::in | std::ios::binary);

	long length = cpp_getfilesize(in);
	GLchar *buf = new GLchar[length+1];
	in.read(buf, length);
	in.close();
	
	buf[length] = '\0';
	

	return buf;
}

GLint checkShader(GLuint *shaderId, GLint QUERY) // QUERY = usually GL_COMPILE_STATUS
{	
	GLint maxLength = 512;
	GLint succeeded;
	glGetShaderiv(*shaderId, QUERY, &succeeded);

	if (!succeeded)
	{
		glGetShaderiv(*shaderId, GL_INFO_LOG_LENGTH, &maxLength);
		char *log = new char[maxLength];
		log[maxLength - 1] = '\0';
			
		glGetShaderInfoLog(*shaderId, maxLength, &maxLength, log);
		printf("[shader status: compile error] see shader.log. \n\n");
		
		std::ofstream logfile("shader.log", std::ios::out | std::ios::app);	
		logfile.write(log, maxLength);
		logfile.close();
		delete [] log;

		return 0;
	}

	else { return 1; }

}


