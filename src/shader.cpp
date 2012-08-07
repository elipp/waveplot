#include "shader.h"


ShaderProgram::ShaderProgram(const std::string& VS_filename, const std::string &FS_filename, const std::string &GS_filename) {

	std::ofstream logfile(Shader::logfilename, std::ios::ate);
	logfile << "";
	logfile.close();

	GLuint VSid, FSid; //, GSid;

	std::ifstream input(VS_filename);
	if (!input.is_open()) {
		printf("ShaderProgram: couldn't open file %s!", VS_filename.c_str());
		error = true;
		return;
	}

	std::stringstream buffer;
	buffer << input.rdbuf();
	std::string VS_contents(buffer.str());
	input.close();
	input.clear();

	buffer.str("");
	buffer.clear();
	input.open(FS_filename);
	if (!input.is_open()) {
		printf("ShaderProgram: couldn't open file %s!", FS_filename.c_str());
		error = true;
		return;
	}
	buffer << input.rdbuf();
	std::string FS_contents(buffer.str());
	
	//printf("%s", fs_contents.c_str());
	if (GS_filename != "") {
		printf("ShaderProgram: geometry shaders not supported yet.\n");
		// do something. don't know how to use geometry shaders yet, so :P
	}

	VSid = glCreateShader(GL_VERTEX_SHADER);
	FSid = glCreateShader(GL_FRAGMENT_SHADER);
	
	const char* VS_p = VS_contents.c_str();
	const char* FS_p = FS_contents.c_str();
	
	const GLint VS_len = VS_contents.length(), 
		        FS_len = FS_contents.length();


	//printf("%d, %d", vs_len, fs_len);

	glShaderSource(VSid, 1, &VS_p, &VS_len);
	glShaderSource(FSid, 1, &FS_p, &FS_len);

	glCompileShader(VSid);
	glCompileShader(FSid);

	if (!Shader::checkShaderCompileStatus(VSid)) {
		printf("Vertex shader compilation failed.\n");
		error = true; return;	
	}
		
	if (!Shader::checkShaderCompileStatus(FSid)) {
		printf("Fragment shader compilation failed.\n");
		error = true; return;
	}

	programHandle_ = glCreateProgram();

	glAttachShader(programHandle_, VSid);
	glAttachShader(programHandle_, FSid);

	glLinkProgram(programHandle_);

	if (!checkLinkStatus()) {
		error = true; return;
	}

	error = false;
}



GLint ShaderProgram::checkLinkStatus() {

	GLint succeeded;
	glGetProgramiv(programHandle_, GL_LINK_STATUS, &succeeded);

	if (!succeeded) {

		printf("[shader status: program %d LINK ERROR!]\n\n", programHandle_);
		error = true;
		return 0;

	}

	else { 
		return 1; 
	}
}

bool ShaderProgram::valid() const {
	return error ? false : true;
}



GLint Shader::checkShaderCompileStatus(GLuint shaderId)
{	

	GLint maxLength = 512;
	GLint succeeded;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &succeeded);

	if (!succeeded)
	{
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
		char *log = new char[maxLength];
		log[maxLength - 1] = '\0';
			
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, log);
		printf("[shader status: compile error] see shader.log.\n\n");
		
		std::ofstream logfile(Shader::logfilename, std::ios::out | std::ios::app);	
		logfile.write(log, maxLength);
		logfile.close();
		delete [] log;

		return 0;
	}

	else { return 1; }
	
}
