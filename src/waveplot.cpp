/* 
 * waveplot.cpp - main file for "waveplot"
 * 
 * While the linux version uses SDL extensively, the windows edition uses the 
 * Win32 API for window and OpenGL/device context creation. It also requires 
 * glew32.dll to work.
 */


#include "gl_includes.h"

#ifdef __linux__
#include <SDL/SDL.h>
#endif

#include <CL/cl.h>	// OpenCL =)

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>


#include "utils.h"
#include "definitions.h"
#include "shader.h"
#include "text.h"
#include "slider.h"
#include "lin_alg.h"

#define BUFFER_OFFSET(i) (reinterpret_cast<void*>(i))

#ifdef _WIN32

static HGLRC hRC = NULL;
static HDC hDC	  = NULL;
static HWND hWnd = NULL;
static HINSTANCE hInstance;

bool active=TRUE;
bool fullscreen=FALSE;
bool keys[256];
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// declare wndproc

#endif

/* 		GL_TRIANGLES SCHEMATIC:	(NEW!)
 *
 *
 *		1---------------------4
 *	        |		    ' |
 *		|		 "    |
 *		|	      '	      |
 *		|	   "	      |
 *		|	'	      |
 *	        |    "		      |
 *	        2`--------------------3 
 *	       
 *	       A counter-clockwise ordering is preferred.
 *
 *
 * */

extern const vertex sliders[];

static float half_WIN_H = (float) WIN_H / 2.0;

static float linewidth = 1.8; 
static float half_linewidth = linewidth/2.0;
static float displacement = 0.0;

static texture gradient_texture, font_texture, slider_texture;

static std::vector<line> lines;
static std::vector<wpstring> strings;

static std::size_t displayrange_left = 0;
static std::size_t displayrange_right = 8*WIN_W;

static GLuint VertexShaderId, FragmentShaderId, programHandle, uniform_texture1_loc, uniform_projection_loc, uniform_modelview_loc;

static bufferObject waveData, sliderData, waveVertexArray;

static float zoom = 0.0;

static mat4 wave_projection, wave_modelview;

namespace Text {
	// glOrtho(0.0, WIN_W, WIN_H, 0.0, 0.0, 1.0);
	mat4 projection_matrix;	// since this will never change.
	mat4 modelview_matrix(MAT_IDENTITY);

}


bool texture::make_texture(const char* filename, GLint filter_flag) {

	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {
		printf("Couldn't open texture file %s\n", filename);
		return false;
	}
	std::size_t filesize = cpp_getfilesize(input);

	char* buffer = new char[filesize];

	input.read(buffer, filesize);
	input.close();

	char* iter = buffer + 2;        // the first two bytes are 'B' and 'M'

	BMPHEADERINFO header;

	memcpy(&header, iter, sizeof(header));

	// validate image OpenGL-wise

	if (header.width == header.height)
	{
		if ((header.width & (header.width - 1)) == 0)   // if power of two
		{
			// image is valid, carry on
			width = height = header.width;
			hasAlpha = header.bpp == 32 ? true : false;

			// read actual image data to buffer.

			GLbyte* imagedata = (GLbyte*)(buffer + 54);

			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &textureId);
			glBindTexture( GL_TEXTURE_2D, textureId);
			glTexImage2D( GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imagedata);

			// GL_MIPMAP_* not accepted for filter_flag, since using it
			// and not actually generating any mipmaps can result in unexpected behavior 
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter_flag );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter_flag );

			delete [] buffer;

			return true;
		}
	}
	return false;

}

void zoomIn() {

	zoom -= 5.0;

	if (displacement > 0.0 && displayrange_left == 0) {
		displayrange_left += 40;
	}

	displayrange_right -= 40;

}

void zoomOut() {

	zoom += 5.0;

	displayrange_left -= 40;
	displayrange_right += 40;

}


void translateLeft() {

	displacement -= 5.0;

	if (!(displacement > 0.0 && displayrange_left == 0)) { 

		displayrange_left += 40;

	}

	displayrange_right += 40;

}

void translateRight() {

	displacement += 5.0;

	if (displayrange_left > 0) {

		displayrange_left -= 40;

	}

	displayrange_right -= 40;

}



line make_line(float x1, float y1, float x2, float y2) {

	line ret_line;
	double dx = (x2 - x1);
	double dy = (y2 - y1);

	y1 = (WIN_H - y1);
	y2 = (WIN_H - y2);

	double slope = dy/dx;
	float angle = atan(slope);

	float xparm = half_linewidth*sin(angle);	// in order to get the actual, rendered ret_line width to match with the specified one,
	float yparm = -half_linewidth*cos(angle);	// the minus is needed since in OpenGL the y-axis is inverted

	//			    x         y        u    v
	ret_line.vertices[0] = vertex(x1-xparm, y1+yparm, 0.0, 0.0);
	ret_line.vertices[1] = vertex(x1+xparm, y1-yparm, 0.0, 1.0);
	ret_line.vertices[2] = vertex(x2+xparm, y2-yparm, 1.0, 1.0);
	ret_line.vertices[3] = vertex(x2-xparm, y2+yparm, 1.0, 0.0);

	return ret_line;

}

void generateWaveVertexArray(triangle* triangles, std::size_t samplecount) {

	const std::size_t trianglecount = (2*samplecount-1);

	glGenBuffers(1, &waveVertexArray.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, waveVertexArray.VBOid);
	glBufferData(GL_ARRAY_BUFFER, (trianglecount)*sizeof(triangle), triangles, GL_STATIC_DRAW);

	waveVertexArray.IBOid = -1;	// not used

}

/*
 * The bakeWaveVertexArray compiles a vertex array of triangle primitives
 * from the input sample data. Rationale:
 *
 * 1) the i915 (Intel GMA) mesa OpenGL-driver apparently doesn't 
 * support GL_UNSIGNED_INT as its index format specifier,
 *
 * 2) the line segments currently share no common vertices (due to poor design),
 * 	- the index buffers obey a strict, predictable sequence. 
 *
 * 3) and glDrawArrays supports rendering by range 
 *
 * this was chosen as the linux implementation. The windows glDrawElements
 * implementation  shall be revamped shortly (to support smooth
 * line segment seams, and thus shared vertices as well)
 *
 *
 * NOTE: as the additional, "smoothing" adjustments (k -> kx, ky) have been introduced,
 * this has become computationally intensive. Consider implementing it in OpenCL.
 */

triangle* bakeWaveVertexArray(float* samples, const std::size_t& samplecount) {

	const std::size_t triangle_count = (samplecount-1)*2;
	triangle* triangles = new triangle[triangle_count];
	const double step = 1.0/8.0;
	double tmpx = 0.0;


	// The first and the last triangles are different from the rest, as they don't have a pair

	float x1 = tmpx;
	float y1 = half_WIN_H*samples[0] + WIN_H;


	float x2 = tmpx + step;
	float y2 = half_WIN_H*samples[1] + WIN_H;

	float x3 = x2 + step;
	float y3 = half_WIN_H*samples[2] + WIN_H;

	y1 = (WIN_H - y1);	// consider these
	y2 = (WIN_H - y2);
	y3 = (WIN_H - y3);
	
	float alpha = atan(step/(y2-y1));
	float beta = atan(step/(y3-y2));

	float xparm = half_linewidth*cos(alpha);	// in order to get the actual, rendered ret_line width to match with the specified one,
	float yparm = -half_linewidth*sin(alpha);	// the minus is needed since in OpenGL the y-axis is inverted

	float gamma_over_2 = 0.5*(M_PI - alpha - beta);

	float k = half_linewidth*tan(gamma_over_2);

	float kx = k*cos(alpha);
	float ky = -k*sin(alpha);			// this too (OpenGL y-axis)

	// lines.push_back(make_line(tmpx, half_WIN_H*samples[i] + WIN_H, tmpx + step, half_WIN_H*samples[i+1] +  WIN_H));
	
	triangles[0].v1 = vertex(x2-xparm+kx, y2 - yparm - ky, 1.0, 0.0);
	triangles[0].v2 = vertex(x2+xparm-kx, y2 + yparm + ky, 1.0, 1.0);
	triangles[0].v3 = vertex(x1, y1, 0.0, 0.5);

	tmpx += step;

	int i = 1, j = 1;

	while (i < triangle_count-1) 
	{

	/*	x1 = tmpx;
		y1 = half_WIN_H*samples[j] + WIN_H; // this could be just copied from the previous result
		x2 = tmpx + step;
		y2 = half_WIN_H*samples[j+1] + WIN_H;	// as well as this
		x3 = x2 + step;
		y3 = half_WIN_H*samples[j+2] + WIN_H; 
		
		THIS IS PREMATURE OPTIMIZATION AT ITS BEST XDDD
		*/
		
		x1 = x2;
		y1 = y2;
		x2 = x3;
		y2 = y3;

		x3 = x2+step;
		y3 = half_WIN_H*samples[j+2] + WIN_H;

		/*y1 = (WIN_H - y1);	// conside4r these
		y2 = (WIN_H - y2); */
		y3 = (WIN_H - y3);

		alpha = atan(step/(y2-y1));	// also could take the previous value :D
		beta = atan(step/(y3-y2));

		xparm = half_linewidth*cos(alpha);
		yparm = -half_linewidth*sin(alpha);	// the minus is needed since in OpenGL the y-axis is inverted
		gamma_over_2 = 0.5*(M_PI - alpha - beta);

		k = half_linewidth*tan(gamma_over_2);


		kx = k*cos(alpha);
		ky = -k*sin(alpha);			// this too has a minus (OpenGL y-axis)

		triangles[i].v1 = triangles[i-1].v2;	// TODO: provide array schematic
		triangles[i].v2 = triangles[i-1].v1;
		triangles[i].v3 = vertex(x2+xparm, y2-yparm, 1.0, 1.0);

		triangles[i+1].v1 = vertex(x2-xparm+kx, y2-yparm-ky, 1.0, 0.0);
		triangles[i+1].v2 = triangles[i].v3;
		triangles[i+1].v3 = triangles[i].v2;

		tmpx += step;

		++j;
		i += 2;

	}
	// as stated above, the last triangle is also different

	triangles[triangle_count-1].v1 = triangles[triangle_count-2].v2;	
	triangles[triangle_count-1].v2 = triangles[triangle_count-2].v1;	
	triangles[triangle_count-1].v3 = vertex(tmpx+step, half_WIN_H*samples[samplecount-1], 1.0, 0.5);

	return triangles;
}




void generateWaveVBOs() {

	glGenBuffers(1, &waveData.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, waveData.VBOid);
	glBufferData(GL_ARRAY_BUFFER, 4*BUFSIZE*sizeof(vertex), &lines[0], GL_STATIC_DRAW);

	const unsigned int num_indices = 6*BUFSIZE;

#ifdef _WIN32
	GLuint *indices = new GLuint[num_indices];	// three indices per triangle, two triangles per line, BUFSIZE lines

#elif __linux__						
	if (BUFSIZE > (0x01<<16)/6) {	// eventually, this will be 0x01<<16/4 (smooth, shared seams)

		/*
		 * TODO: Split VBOs into chunks of BUFSIZE/4
		 */
		// std::cout << "Buffer size is over 16384. Expect problems.";
		std::cout << "Buffer size is over 10922 (" << num_indices <<"). Expect problems.";


	}

	GLushort *indices = new GLushort[num_indices];	// apparently, the linux mesa driver doesn't support GL_UNSIGNED_INT
#endif

	int i = 0;
	int j = 0;


	while (i < num_indices) {

		indices[i] = j;
		indices[i+1] = j+1;
		indices[i+2] = j+3;
		indices[i+3] = j+1;
		indices[i+4] = j+2;
		indices[i+5] = j+3;

		i += 6;
		j += 4;


	}

	//debug

	glGenBuffers(1, &waveData.IBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waveData.IBOid);
#ifdef _WIN32
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*(sizeof(GLuint)), indices, GL_STATIC_DRAW);
#elif __linux__
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices*(sizeof(GLushort)), indices, GL_STATIC_DRAW);
#endif

	delete [] indices;


}


bool InitGL()
{
	
	GLenum err;

	#ifdef _WIN32
	err = glewInit();

	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		printf("Error: %s\n", glewGetErrorString(err));
		return false;
	}

	#endif

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,WIN_W,WIN_H, 0,0.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();*/
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	const char* version = (const char*) glGetString(GL_VERSION);

	printf("\nOpenGL version information:\n%s\n\n", version);

	bool gradient_texture_valid = gradient_texture.make_texture("textures/texture.bmp", GL_LINEAR);
	bool font_texture_valid = font_texture.make_texture("textures/dina_all.bmp", GL_NEAREST);
	bool slider_texture_valid = slider_texture.make_texture("textures/slider.bmp", GL_NEAREST);

	if (!(gradient_texture_valid && font_texture_valid && slider_texture_valid)) {
		printf("Failure loading textures.");
		return false;
	}

#ifdef _WIN32
	const char* vshadername = "shaders/vertex.shader.win";
	const char* fshadername = "shaders/fragment.shader.win";

#elif __linux__
	const char* vshadername = "shaders/vertex.shader.linux";
	const char* fshadername = "shaders/fragment.shader.linux";
#endif

	GLchar* vert_shader = readShaderFromFile(vshadername);
	GLchar* frag_shader = readShaderFromFile(fshadername);

	GLint vlen = strlen(vert_shader);
	GLint flen = strlen(frag_shader);

	VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(VertexShaderId, 1, (const GLchar**)&vert_shader, &vlen);
	glShaderSource(FragmentShaderId, 1, (const GLchar**)&frag_shader,&flen);

	glCompileShader(VertexShaderId);
	glCompileShader(FragmentShaderId);

	if (!(checkShaderCompileStatus(FragmentShaderId) && checkShaderCompileStatus(VertexShaderId)))
	{
		printf("Shader compile error. See shader.log\n");
		return false;
	}

	programHandle = glCreateProgram();

	glAttachShader(programHandle, VertexShaderId);
	glAttachShader(programHandle, FragmentShaderId);

	glLinkProgram(programHandle);
	
	if (checkProgramLinkStatus(programHandle) == FALSE){
		printf("Shader LINK error.\n");
		return false;
	}
	
	glUseProgram(programHandle);


	delete [] vert_shader;
	delete [] frag_shader;

	


#ifdef _WIN32

	glBindAttribLocation(programHandle, 0, "in_position");
	glBindAttribLocation(programHandle, 1, "in_texcoord");
	glBindFragDataLocation(programHandle, 0, "out_fragcolor");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	

#endif
	
	uniform_texture1_loc = glGetUniformLocation(programHandle, "texture_1");
	
	uniform_projection_loc = glGetUniformLocation(programHandle, "projectionMatrix");
	uniform_modelview_loc = glGetUniformLocation(programHandle, "modelviewMatrix");
	
	err = glGetError();

	if (err != GL_NO_ERROR) {

#ifdef _WIN32
		printf("gl error detected. Error code: %d", (int)err);
#elif __linux__
		std::cout << "gl error detected. Error code: " << (int) err;
#endif
		return false;

	}

	printf("%d %d %d\n", uniform_texture1_loc, uniform_projection_loc, uniform_modelview_loc);

	// these two cause problemz
	//generateWaveVBOs();
	//generateSliderVBOs();

	return true;
}


void drawSliders() {
	
	glBindBuffer(GL_ARRAY_BUFFER, sliderData.VBOid);

#ifdef _WIN32

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__	// intel i915 only supports OpenGL up to 1.4 (mesa 8)

	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));

#endif
	glUniform1i(uniform_texture1_loc, 0);

	/*glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, WIN_W, WIN_H, 0.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); */

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliderData.IBOid);
	glUseProgram(programHandle);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, slider_texture.textureId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliderData.IBOid);
	glDrawElements(GL_TRIANGLES, 11*2, GL_UNSIGNED_SHORT, NULL);

}

void drawWave() {


	glBindBuffer(GL_ARRAY_BUFFER, waveData.VBOid);

#ifdef _WIN32

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__	// intel i915 only supports OpenGL up to 1.4 (mesa 8)

	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));

#endif

	wave_projection.make_proj_orthographic(-zoom*aspect_ratio, WIN_W+zoom*aspect_ratio, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), -1.0f, 1.0f);
	wave_modelview.identity();
	// no translation facilities xDD
	wave_modelview(3,0) = displacement;
	glUseProgram(programHandle);
	glUniform1i(uniform_texture1_loc, 0);
	glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, wave_projection.rawdata());
	glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, wave_modelview.rawdata());
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waveData.IBOid);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);
#ifdef _WIN32
	glDrawElements(GL_TRIANGLES, BUFSIZE*2, GL_UNSIGNED_INT, NULL);
#elif __linux__
	glDrawElements(GL_TRIANGLES, BUFSIZE/4, GL_UNSIGNED_SHORT, NULL);
#endif
	
	glUseProgram(0);
	
	//glPopMatrix();
}


void drawWaveVertexArray() {

	glBindBuffer(GL_ARRAY_BUFFER, waveVertexArray.VBOid); 

	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glUniform1i(uniform_texture1_loc, 0);

/*	glMatrixMode(GL_PROJECTION);
	
	glOrtho(-zoom*aspect_ratio, WIN_W+zoom*aspect_ratio, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(displacement, 0.0, 0.0);

	glUseProgram(programHandle);*/

	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);
	glDrawArrays(GL_TRIANGLES, 0, BUFSIZE/2);
//	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

}

void drawText() {

	static std::vector<wpstring>::const_iterator iter;

	// damn static variables.
	
	iter = strings.begin();
	
	while(iter != strings.end()) {
		
		glBindBuffer(GL_ARRAY_BUFFER, (*iter).bufObj.VBOid);
#ifdef _WIN32
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));
#endif
		Text::projection_matrix.make_proj_orthographic(0.0, WIN_W, WIN_H, 0.0, -1.0, 1.0);
		Text::modelview_matrix.identity();


		glUseProgram(programHandle);
		glUniform1i(uniform_texture1_loc, 0);
		
		glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, Text::projection_matrix.rawdata());
		glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, Text::modelview_matrix.rawdata());

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*iter).bufObj.IBOid);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font_texture.textureId);

		glDrawElements(GL_TRIANGLES, 6*(*iter).length, GL_UNSIGNED_SHORT, NULL);
		
		glUseProgram(0);
		++iter;
	}



}

inline void draw() {

	glClear(GL_COLOR_BUFFER_BIT);

	drawWave();
	//fdrawWaveVertexArray();
	drawText();
	//drawSliders();
		
}

#ifdef _WIN32

void KillGLWindow(void)
{
	if(hRC)
	{
		if(!wglMakeCurrent(NULL,NULL))
		{
			MessageBox(NULL, "wglMakeCurrent(NULL,NULL) failed", "erreur", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))
		{
			MessageBox(NULL, "RELEASE of rendering context failed.", "error", MB_OK | MB_ICONINFORMATION);
		}
		hRC=NULL;

		if(hDC && !ReleaseDC(hWnd, hDC))
		{
			MessageBox(NULL, "Release DC failed.", "ERREUX", MB_OK | MB_ICONINFORMATION);
			hDC=NULL;
		}

		if(hWnd && !DestroyWindow(hWnd))
		{
			MessageBox(NULL, "couldn't release hWnd.", "erruexz", MB_OK|MB_ICONINFORMATION);
			hWnd=NULL;
		}

		if (!UnregisterClass("OpenGL", hInstance))
		{
			MessageBox(NULL, "couldn't unregister class.", "err", MB_OK | MB_ICONINFORMATION);
			hInstance=NULL;
		}

	}

}


BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;

	RECT WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)width;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)height;

	fullscreen = fullscreenflag;

	hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC) WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "OpenGL";

	if (!RegisterClass(&wc))
	{
		MessageBox(NULL, "FAILED TO REGISTER THE WINDOW CLASS.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}

	DEVMODE dmScreenSettings;
	memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
	dmScreenSettings.dmSize = sizeof(dmScreenSettings);
	dmScreenSettings.dmPelsWidth = width;
	dmScreenSettings.dmPelsHeight = height;
	dmScreenSettings.dmBitsPerPel = bits;
	dmScreenSettings.dmFields= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

	/*
	 * no need to test this now that fullscreen is turned off
	 *
	 if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	 {
	 if (MessageBox(NULL, "The requested fullscreen mode is not supported by\nyour video card. Use Windowed mode instead?", "warn", MB_YESNO | MB_ICONEXCLAMATION)==IDYES)
	 {
	 fullscreen=FALSE;
	 }
	 else {

	 MessageBox(NULL, "Program willl now close.", "ERROR", MB_OK|MB_ICONSTOP);
	 return FALSE;
	 }
	 }*/

	ShowCursor(TRUE);
	if (fullscreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;

	}

	else {
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if(!(hWnd=CreateWindowEx( dwExStyle, "OpenGL", title,
					WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
					0, 0,
					WindowRect.right-WindowRect.left,
					WindowRect.bottom-WindowRect.top,
					NULL,
					NULL,
					hInstance,
					NULL)))
	{
		KillGLWindow();
		MessageBox(NULL, "window creation error.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}


	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		bits,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	if (!(hDC=GetDC(hWnd)))
	{
		KillGLWindow();
		MessageBox(NULL, "CANT CREATE A GL DEVICE CONTEXT.", "ERROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		KillGLWindow();
		MessageBox(NULL, "cant find a suitable pixelformat.", "ERROUE", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}


	if(!SetPixelFormat(hDC, PixelFormat, &pfd))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't SET ZE PIXEL FORMAT.", "ERROU", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!(hRC=wglCreateContext(hDC)))
	{
		KillGLWindow();
		MessageBox(NULL, "WGLCREATECONTEXT FAILED.", "ERREUHX", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	if(!wglMakeCurrent(hDC, hRC))
	{
		KillGLWindow();
		MessageBox(NULL, "Can't activate the gl rendering context.", "ERAIX", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	ShowWindow(hWnd, SW_SHOW);
	SetForegroundWindow(hWnd);
	SetFocus(hWnd);

	if (!InitGL())
	{
		KillGLWindow();
		MessageBox(NULL, "InitGL() failed.", "ERRROR", MB_OK|MB_ICONEXCLAMATION);
		return FALSE;
	}

	return TRUE;
}


LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_ACTIVATE:
			if(!HIWORD(wParam))
			{
				active=TRUE;
			}

			else 
			{
				active=FALSE;
			}
			return 0;

		case WM_SYSCOMMAND:
			switch(wParam)
			{
				case SC_SCREENSAVE:
				case SC_MONITORPOWER:
					return 0;
			}
			break;

		case WM_CLOSE:
			{
				PostQuitMessage(0);
				return 0;
			}

		case WM_KEYDOWN:
			{
				keys[wParam]=TRUE;
				return 0;
			}
		case WM_KEYUP:
			{
				keys[wParam]=FALSE;
				return 0;
			}
		case WM_SIZE:
			{
				//ResizeGLScene(LOWORD(lParam), HIWORD(lParam));
			}
			;
	}

	/* the rest shall be passed to defwindowproc. (default window procedure) */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	/* allocate console for debug output (only works with printf doe) */

	if(AllocConsole()) {
		freopen("CONOUT$", "wt", stdout);
		SetConsoleTitle("debug output");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED);
	}


	MSG msg;
	BOOL done=FALSE;

	fullscreen=FALSE;

	if (!CreateGLWindow("waveplot", WIN_W, WIN_H, 32, FALSE)) {
		return 1;
	}

	int running=1;

	const char* filename = "resources/asdfmono.wav";

	std::string string1 = std::string("Filename: ") + std::string(filename);
	strings.push_back(wpstring(string1, 15, 15));

	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {	
		printf("Couldn't open resource file %s\n", filename);
		return 1;

	}

	std::size_t num_samples;

	// the readSampleData_int16 function actually reads the whole file.
	float *samples = readSampleData_int16(input, &num_samples);	// presuming 16-bit, little endian

	num_samples = (num_samples < BUFSIZE) ? num_samples : (std::size_t) BUFSIZE;

	triangle *triangles = bakeWaveVertexArray(samples, num_samples);
	
	generateWaveVertexArray(triangles, num_samples);
	delete [] triangles;

	float tmpx = 0.0;
	static float step = 0.125;

	for (int i=0; i < BUFSIZE; i++)
	{
		lines.push_back(make_line(tmpx, half_WIN_H*(samples[i]+1.0), tmpx + step, half_WIN_H*(samples[i+1]+1.0)));
		tmpx+=step;
	}
	
	generateWaveVBOs();
	//generateSliderVBOs();
	delete [] samples;

	while(!done)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				done=TRUE;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			if (active)
			{
				if(keys[VK_ESCAPE])
				{
					done = true;
				}

				if(keys[VK_UP]) {
					zoomIn();
					keys[VK_UP] = false;
				}
				if(keys[VK_DOWN]) {
					zoomOut();
					keys[VK_DOWN] = false;
				}
				if(keys[VK_RIGHT]) {
					translateRight();
					keys[VK_RIGHT] = false;
				}
				if(keys[VK_LEFT]) {
					translateLeft();
					keys[VK_LEFT] = false;
				}

				draw();
				SwapBuffers(hDC);

			}
		}

	}

	KillGLWindow();
	glDeleteBuffers(1, &waveData.VBOid);
	glDeleteBuffers(1, &waveData.IBOid);
	return (msg.wParam);
}

#elif __linux__

SDL_Surface* createSDLWindow() {

	SDL_Init(SDL_INIT_VIDEO);
	SDL_WM_SetCaption("waveplot", NULL);
	SDL_Surface *screen=SDL_SetVideoMode(WIN_W,WIN_H,32, SDL_HWSURFACE | SDL_OPENGL);

	return screen;
}

int main(int argc, char *argv[])
{
	int running=1;
	Uint32 start;
	SDL_Event event;

	if (argc < 2) { 

		std::cout << "No input.\n";
		return 1;

	}
	const char* filename = argv[1];

	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {
		std::cout << "Couldn't open file " << filename << ": no such file or directory\n";
		return 1;

	}

	std::size_t num_samples;

	// the readSampleData_int16 function actually reads the whole file.
	float *samples = readSampleData_int16(input, &num_samples);	// presuming 16-bit, little endian

	num_samples = (num_samples < BUFSIZE) ? num_samples : (std::size_t) BUFSIZE;

	triangle *triangles = bakeWaveVertexArray(samples, num_samples);

	float tmpx = 0.0;
	static float step = 0.125;

	for (int i=0; i < BUFSIZE; i++)
	{
		lines.push_back(make_line(tmpx, (WIN_H/2)*samples[i] + WIN_H, tmpx + step, (WIN_H/2)*samples[i+1] + WIN_H));
		tmpx+=step;
	}

	delete [] samples;

	SDL_Surface *screen = createSDLWindow();	// we should now have a GL context.
	if (!screen) {
		std::cout << "Couldn't create SDL window.";
		return 1;
	}

	if(!InitGL()) {

		std::cout << "InitGL failed.\n";
		return 1;

	}

	std::string string1 = std::string("Filename: ") + std::string(filename);
	strings.push_back(wpstring(string1, 15, 15));

	generateWaveVertexArray(triangles, num_samples);

	delete [] triangles;

	draw();
	SDL_GL_SwapBuffers();

	/* LIN_ALG testing sandbox BEGIN */

	float m[16];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-zoom*aspect_ratio, WIN_W+zoom*aspect_ratio, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), 0.0f, 1.0f);
	//glOrtho(0, WIN_W, WIN_H, 0, 0.0, 1.0);
	glGetFloatv(GL_PROJECTION_MATRIX, m);

	mat4 u(m);
	u.print();

	vec4 a(0.15, 300.5, 0.0, 1.0);

	vec4 trans=ftransform_test(a);
	trans.print();

	/* LIN_ALG testing sandbox END */

	while(running)
	{
		while(SDL_PollEvent(&event) && running)
		{
			switch(event.type)
			{
				case SDL_QUIT:
					running=0;
					break;

				case SDL_KEYDOWN:

					switch(event.key.keysym.sym)
					{
						case SDLK_ESCAPE:
							running=0;
							break;

						case SDLK_UP:
							zoomIn();
							draw();
							SDL_GL_SwapBuffers();
							break;
						case SDLK_DOWN:
							zoomOut();
							draw();
							SDL_GL_SwapBuffers();
							break;
						case SDLK_LEFT:
							translateLeft();
							draw();
							SDL_GL_SwapBuffers();
							break;
						case SDLK_RIGHT:
							translateRight();
							draw();
							SDL_GL_SwapBuffers();
							break;
						default:
							;
					}
					break;

				default:
					break;
			} 


		}

	}

	SDL_Quit();
	return 0;
}


#endif
