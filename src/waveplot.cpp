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

#ifdef _WIN32 
#include <process.h>
#endif


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
#include "timer.h"

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
 *		1---------------------3
 *	        |		    ' |
 *		|		 "    |
 *		|	      '	      |
 *		|	   "	      |
 *		|	'	      |
 *	        |    "		      |
 *	        2`--------------------4 
 *	       
 *	       A counter-clockwise ordering is preferred.
 *
 *
 * */


static const vertex fullscreen_quad_vertices[6] = { 
				vertex(-1.0, -1.0, 0.0, 0.0),
				vertex(1.0, -1.0, 1.0, 0.0),
				vertex(1.0, 1.0, 1.0, 1.0),
				vertex(-1.0, 1.0, 0.0, 1.0),
				vertex(-1.0, -1.0, 0.0, 0.0),
				vertex(1.0, 1.0, 1.0, 1.0)
};

static std::string input_filename("resources/asdfmono.wav");

extern const vertex sliders[];

static float half_WIN_H = (float) WIN_H / 2.0;

static float linewidth = 1.8; 
static float half_linewidth = linewidth/2.0;

static texture gradient_texture, font_texture, slider_texture, solid_color_texture;

static std::vector<line> lines;

static GLuint uniform_texture1_loc, uniform_projection_loc, uniform_modelview_loc;
static GLuint uniform_texture1_loc_fullscreen_quad;
static bufferObject waveData, sliderData, waveVertexArray, fullscreen_quadData;
static mat4 wave_projection, wave_modelview;
static int wave_polygonMode = GL_FILL;
static bool wave_solidColorTextureToggle = false;
static const double dx = 1.0/4.0;
static const double frame_interval = 1.0/60.0;	// actually, handled by hardware vsync on my machine

static ShaderProgram *passthrough_shader_program = NULL;
static ShaderProgram *fullscreen_quad_shader = NULL;

namespace Text {
	mat4 projection_matrix;
	mat4 modelview_matrix(MAT_IDENTITY);
}

namespace View {
	static bool mbuttondown = false;
	static LPPOINT mouse_pos0 = new POINT;
	static LPPOINT prev_mouse_pos = new POINT;
	static vec4 wave_pos0;
	static RECT windowRect;
	static float dx, dy, prev_dx, prev_dy;
	
	static float zoom = 0.0;
	static const float zoom_step = 10.0, zoom_min = -64*zoom_step, zoom_max = 24*zoom_step;
	
	static vec4 wave_position, // constructed as zero vectors.
			wave_view_velocity,// used to give the notion of inertia to the motion of the camera
			wave_view_velocity_sample1;
	
	static Quaternion rot;	// initialized as the identity quaternion (0, 0, 0, 1)

	void zoomIn();
	void zoomOut();
}

void View::zoomIn() {

	View::zoom = View::zoom <= View::zoom_min ? View::zoom_min : View::zoom - View::zoom_step;

}

void View::zoomOut() {
	View::zoom = View::zoom >= View::zoom_max ? View::zoom_max : View::zoom + View::zoom_step;
}

static GLuint FBOid, FBOtextureid;	// for post-processing


bool texture::make_texture(const std::string& filename, GLint filter_flag) {

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

GLuint generateWaveVertexBufferObject(vertex* vertices) {

	const std::size_t vertex_count = (2*BUFSIZE-2);

	GLuint ret;
	glGenBuffers(1, &ret);
	glBindBuffer(GL_ARRAY_BUFFER, ret);
	glBufferData(GL_ARRAY_BUFFER, (vertex_count)*sizeof(vertex), (const GLvoid*)vertices, GL_STATIC_DRAW);

	return ret;

}

GLuint generateGlobalIndexBuffer(GLuint *indices) {

	GLuint ret;
	glGenBuffers(1, &ret);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, BUFSIZE_MAX*sizeof(GLuint), (const GLvoid*)indices, GL_STATIC_DRAW);

	return ret;

}

void destroyCurrentWaveVertexBuffer() {

	glDeleteBuffers(1, &waveData.VBOid);
	// the vertex buffer has a static IBO, allocated to BUFSIZE_MAX
	//glDeleteBuffers(1, &waveVertexArray.IBOid);

}

// The drawback of this algorithm is that some of the
// really "tight turns" in the waveform produce unwanted artifacts.

triangle *bakeWaveVertexArrayUsingLineIntersections(float* samples, const std::size_t& samplecount) {
	
	const std::size_t triangle_count = 2*samplecount-1;
	triangle* triangles = new triangle[triangle_count];

	static const float h = half_linewidth;

	float x1 = 0.0;
	float y1 = half_WIN_H*samples[0] + half_WIN_H;

	float x2 = x1 + dx;
	float y2 = half_WIN_H*samples[1] + half_WIN_H;

	float x3 = x2 + dx;
	float y3 = half_WIN_H*samples[2] + half_WIN_H;

	float k1 = (y2-y1)/dx;
	float alpha_1 = atan(k1);
	
	float k2 = (y3-y2)/dx;
	float alpha_2 = atan(k2);

	float px_2 = h*sin(alpha_1);
	float py_2 = h*cos(alpha_1);

	triangles[0].v1 = vertex(x2+px_2, WIN_H - (y2-py_2), 1.0, 0.0);
	triangles[0].v2 = vertex(x2-px_2, WIN_H - (y2+py_2), 1.0, 1.0);
	triangles[0].v3 = vertex(x1, y1, 0.0, 0.5);
	
	int i = 1, j = 1;

	float x2_c, y2_c;
	float dk;
	float px_3, py_3;
	float res_x2_1, res_y2_1, res_x2_2, res_y2_2;

	while (i < triangle_count - 1) 
	{
		//    y - y0 = k(x - x0)
		// =>      y = k(x - x0) + y0

		x1 = x2; y1 = y2;
		x2 = x3; y2 = y3;
		x3 = x2+dx;
		y3 = half_WIN_H*samples[j+2] + half_WIN_H;

		k1 = (y2-y1)/dx;	// dx = constant
		alpha_1 = atan(k1);	// can be copied from previous result
							// also, this temporary isn't necessary
	
		k2 = (y3-y2)/dx;
		alpha_2 = atan(k2);

		px_2 = h*sin(alpha_1);
		py_2 = h*cos(alpha_1);
		
		px_3 = h*sin(alpha_2);
		py_3 = h*cos(alpha_2);

		dk = k1-k2;

		if (fabs(dk) < 0.3) {

			res_x2_1 = x2 - px_2;
			res_y2_1 = y2 + py_2;

			res_x2_2 = x2 + px_2;
			res_y2_2 = y2 - py_2;

		}

		else {
			
			x2_c = (x2 - px_2);
			y2_c = (y2 + py_2);
			
			res_x2_1 = (k1*x2_c - y2_c - k2*(x3-px_3) + (y3+py_3))/dk;
			res_y2_1 = k1*(res_x2_1 - x2_c) + y2_c;
		
			x2_c = (x2 + px_2);
			y2_c = (y2 - py_2);

			res_x2_2 = (k1*x2_c - y2_c - k2*(x3+px_3) + (y3-py_3))/dk;
			res_y2_2 = k1*(res_x2_2 - x2_c) + y2_c;
		
		}
		
		// invert computed y values

		res_y2_1 = (WIN_H - res_y2_1);
		res_y2_2 = (WIN_H - res_y2_2);	
	
		triangles[i].v1 = triangles[i-1].v2;
		triangles[i].v2 = triangles[i-1].v1;
		triangles[i].v3 = vertex(res_x2_1, res_y2_1, 1.0, 1.0);

		triangles[i+1].v1 = vertex(res_x2_2, res_y2_2, 1.0, 0.0);
		triangles[i+1].v2 = triangles[i].v3;
		triangles[i+1].v3 = triangles[i].v2;
		
		++j;
		i += 2;

	}

	triangles[triangle_count-1].v1 = triangles[triangle_count-2].v2;	
	triangles[triangle_count-1].v2 = triangles[triangle_count-2].v1;	
	triangles[triangle_count-1].v3 = vertex(triangles[triangle_count-2].v1.x()+dx, half_WIN_H*samples[samplecount-1] + half_WIN_H, 1.0, 0.5);

	return triangles;
}


vertex* bakeWaveVertexBufferUsingLineIntersections(const float* samples, const std::size_t& samplecount) {
	
	const std::size_t vertex_count = 2*samplecount-2;
	vertex* vertices = new vertex[vertex_count];
	static const float h = half_linewidth;

	float x1 = 0.0;
	float y1 = half_WIN_H*samples[0] + half_WIN_H;

	float x2 = x1 + dx;
	float y2 = half_WIN_H*samples[1] + half_WIN_H;

	float x3 = x2 + dx;
	float y3 = half_WIN_H*samples[2] + half_WIN_H;

	float k1 = (y2-y1)/dx;
	float alpha_1 = atan(k1);
	
	float k2 = (y3-y2)/dx;
	float alpha_2 = atan(k2);

	float px_2 = h*sin(alpha_2);
	float py_2 = h*cos(alpha_2);

	// special case
	vertices[0] = vertex(x1, WIN_H - y1, 0.0, 0.5);
	vertices[2] = vertex(x2+px_2, WIN_H - (y2-py_2), 0.0, 0.0);
	vertices[1] = vertex(x2-px_2, WIN_H - (y2+py_2), 0.0, 1.0);
		
	int i = 3, j = 3;
	
	float alpha_3 = atan(((half_WIN_H*samples[3] + half_WIN_H)-y3)/dx);
	float x2_c, y2_c;
	float dk;
	float px_3 = h*sin(alpha_3), py_3 = h*cos(alpha_3);
	float res_x2_1, res_y2_1, res_x2_2, res_y2_2;

	while (i < vertex_count - 1) 
	{
		//    y - y0 = k(x - x0)
		// =>      y = k(x - x0) + y0

		x1 = x2; y1 = y2;
		x2 = x3; y2 = y3;
		x3 = x2+dx;
		y3 = half_WIN_H*samples[j] + half_WIN_H;

		//k1 = (y2-y1)/dx;	// dx = constant
		k1 = k2;
		alpha_1 = alpha_2;	// can be copied from previous result
							// also, this temporary isn't necessary
	
		k2 = (y3-y2)/dx;
		alpha_2 = atan(k2);

		//px_2 = h*sin(alpha_1);
		//py_2 = h*cos(alpha_1);
		
		px_2 = px_3;
		py_2 = py_3;

		px_3 = h*sin(alpha_2);
		py_3 = h*cos(alpha_2);

		dk = k1-k2;

		if (fabs(dk) < 0.3) {

			res_x2_1 = x2 - px_2;
			res_y2_1 = y2 + py_2;

			res_x2_2 = x2 + px_2;
			res_y2_2 = y2 - py_2;

		}

		else {
			
			// calculate line intersections
			
			//    y - y0 = k(x - x0)
			// =>      y = k(x - x0) + y0
			
			//	  set y1 = y2 
			// => k1(x - x1) + y01 = k2(x - x2) + y02
			//
			// solving for x yields:
			// x = (k1x1 - y01 - k2x2 + y02)/(k1 - k2).
			// then solve for y.

			x2_c = (x2 - px_2);
			y2_c = (y2 + py_2);
			
			res_x2_1 = (k1*x2_c - y2_c - k2*(x3-px_3) + (y3+py_3))/dk;
			res_y2_1 = k1*(res_x2_1 - x2_c) + y2_c;
		
			x2_c = (x2 + px_2);
			y2_c = (y2 - py_2);

			res_x2_2 = (k1*x2_c - y2_c - k2*(x3+px_3) + (y3-py_3))/dk;
			res_y2_2 = k1*(res_x2_2 - x2_c) + y2_c;
		
		}
		
		// invert computed y values

		res_y2_1 = (WIN_H - res_y2_1);
		res_y2_2 = (WIN_H - res_y2_2);	
	
		vertices[i] = vertex(res_x2_1, res_y2_1, 1.0, 1.0); 
		vertices[i+1] = vertex(res_x2_2, res_y2_2, 1.0, 0.0);
		
		
		++j;
		i += 2;

	}
	// these are still bugged
	vertices[vertex_count-2] = vertex(30000, 0, 0, 0);
	vertices[vertex_count-1] = vertex(vertices[vertex_count-2].x()+dx, half_WIN_H*samples[samplecount-1] + half_WIN_H, 1.0, 0.5);

	return vertices;

}


GLuint *generateIndexBufferWithSharedVertices() {

	// just use BUFSIZE_MAX :D
	GLuint *indexBuffer = new GLuint[BUFSIZE_MAX];

	indexBuffer[0] = 0;
	indexBuffer[1] = 2;
	indexBuffer[2] = 1;

	int i = 3, j = 1;

	while (i < BUFSIZE_MAX - 6) {

		// 1, 2, 3, 4, 3, 2

		indexBuffer[i] = j;
		indexBuffer[i+1] = j+1;
		indexBuffer[i+2] = j+2;

		indexBuffer[i+3] = j+3;
		indexBuffer[i+4] = j+2;
		indexBuffer[i+5] = j+1;

		i += 6;
		j += 2;

	}
	// this is probably wrong
	indexBuffer[i] = j;
	indexBuffer[i+1] = j+1;
	indexBuffer[i+2] = j+2;

	return indexBuffer;

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
	glViewport(0, 0, WIN_W, WIN_H);

	const char* version = (const char*) glGetString(GL_VERSION);

	printf("\nOpenGL version information:\n%s\n\n", version);

	GLint max_elements_vertices, max_elements_indices;
	glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &max_elements_vertices);
	glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &max_elements_indices);

	printf("GL_MAX_ELEMENTS_VERTICES = %d\nGL_MAX_ELEMENTS_INDICES = %d\n", max_elements_vertices, max_elements_indices);

	bool gradient_texture_valid = gradient_texture.make_texture("textures/gradient.bmp", GL_LINEAR);	// solid_color_test.bmp
	bool font_texture_valid = font_texture.make_texture("textures/dina_all.bmp", GL_NEAREST);
	bool slider_texture_valid = slider_texture.make_texture("textures/slider.bmp", GL_NEAREST);
	bool solid_color_texture_valid = solid_color_texture.make_texture("textures/solid_color_test.bmp", GL_LINEAR);

	if (!(gradient_texture_valid && font_texture_valid && slider_texture_valid && solid_color_texture_valid)) {
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

	passthrough_shader_program = new ShaderProgram("shaders/vertex.shader.win", "shaders/fragment.shader.win", "");
	fullscreen_quad_shader = new ShaderProgram("shaders/quad_passthrough.shader.win", "shaders/quad_fragment.shader.win", "");
	
	if (!passthrough_shader_program->valid()) {
		delete passthrough_shader_program;
		return false;
	}
	if (!fullscreen_quad_shader->valid()) {
		delete fullscreen_quad_shader;
		return false;
	}

	glUseProgram(passthrough_shader_program->programHandle());

	Text::projection_matrix.make_proj_orthographic(0.0, WIN_W, WIN_H, 0.0, -1.0, 1.0);
	Text::modelview_matrix.identity();


#ifdef _WIN32

	glBindAttribLocation(passthrough_shader_program->programHandle(), 0, "in_position");
	glBindAttribLocation(passthrough_shader_program->programHandle(), 1, "in_texcoord");
	glBindFragDataLocation(passthrough_shader_program->programHandle(), 0, "out_fragcolor");

	glBindAttribLocation(fullscreen_quad_shader->programHandle(), 0, "in_position");
	glBindAttribLocation(fullscreen_quad_shader->programHandle(), 1, "in_texcoord");
	glBindFragDataLocation(fullscreen_quad_shader->programHandle(), 0, "out_fragcolor");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	

#endif
	
	uniform_texture1_loc = glGetUniformLocation(passthrough_shader_program->programHandle(), "texture_1");
	uniform_texture1_loc_fullscreen_quad = glGetUniformLocation(fullscreen_quad_shader->programHandle(), "texture_1");

	uniform_projection_loc = glGetUniformLocation(passthrough_shader_program->programHandle(), "projectionMatrix");
	uniform_modelview_loc = glGetUniformLocation(passthrough_shader_program->programHandle(), "modelviewMatrix");
	
	printf("%d\n", uniform_texture1_loc_fullscreen_quad);

	err = glGetError();

	if (err != GL_NO_ERROR) {

#ifdef _WIN32
		printf("gl error detected. Error code: %d", (int)err);
#elif __linux__
		std::cout << "gl error detected. Error code: " << (int) err;
#endif
		return false;

	}

	//wglSwapIntervalEXT(0);

	// in terms of smoothness, hardware "Always on" VSYNC yields the best results (at least for me, Radeon HD 5770)

	glGenFramebuffers(1, &FBOid);
	glBindFramebuffer(GL_FRAMEBUFFER, FBOid);
//
	glGenTextures(1, &FBOtextureid);
	glBindTexture(GL_TEXTURE_2D, FBOtextureid);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIN_W, WIN_H, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, FBOtextureid, 0);

	GLenum m[2] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, m);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {

		MessageBox(hWnd, "FBO initialization failed!", "error", NULL);
		return false;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, FBOid);

	glGenBuffers(1, &fullscreen_quadData.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quadData.VBOid);
	glBufferData(GL_ARRAY_BUFFER, 6*sizeof(vertex), fullscreen_quad_vertices, GL_STATIC_DRAW);

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
	glUseProgram(passthrough_shader_program->programHandle());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, slider_texture.textureId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sliderData.IBOid);
	glDrawElements(GL_TRIANGLES, 11*2, GL_UNSIGNED_SHORT, NULL);

}

void drawWave() {
	
	glPolygonMode(GL_FRONT_AND_BACK, wave_polygonMode);
	glBindBuffer(GL_ARRAY_BUFFER, waveData.VBOid);

#ifdef _WIN32

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__	// intel i915 only supports OpenGL up to 1.4 (mesa 8)

	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));

#endif

	wave_projection.make_proj_orthographic(-View::zoom, WIN_W+View::zoom, WIN_H+(View::zoom*aspect_ratio_recip), -(View::zoom*aspect_ratio_recip), -1.0f, 1.0f);
	wave_modelview.identity();
	wave_modelview(3,0) = View::wave_position(0);
	wave_modelview(3,1) = View::wave_position(1);
	glUseProgram(passthrough_shader_program->programHandle());
	glUniform1i(uniform_texture1_loc, 0);
	glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, (const GLfloat*)wave_projection.rawData());
	glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, (const GLfloat*)wave_modelview.rawData());
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waveData.IBOid);

	glActiveTexture(GL_TEXTURE0);
	
	if (wave_solidColorTextureToggle) {
		glBindTexture(GL_TEXTURE_2D, solid_color_texture.textureId);
	} else {		
		glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);
	}
#ifdef _WIN32

	static const int spp = 1.0/dx;	// samples per pixel

	int samples_shown = (WIN_W + 2*View::zoom)*spp;
	int actual_offset = -(int)(View::wave_position(0)) - (int)View::zoom;
	if (actual_offset < 0) actual_offset = 0;
	if (actual_offset*dx + samples_shown > BUFSIZE) {
		samples_shown -= spp*actual_offset + samples_shown - BUFSIZE + 4;	// the 4 is just an arbitrary constant
		if (samples_shown < 0) samples_shown = 0;							// to "complement" known bakeWaveVertexBuffer...()
	}																		// end irregularities 
																			// (i.e. conceal the manifestation of a bug. :P)

	// when pos < 0, the program still renders samples_shown 
	// samples even though they're out of the field of view.
	
	glDrawElements(GL_TRIANGLES, 6*samples_shown, GL_UNSIGNED_INT, BUFFER_OFFSET(6*spp*(actual_offset)*sizeof(GLuint)));
#elif __linux__
	glDrawElements(GL_TRIANGLES, BUFSIZE*2, GL_UNSIGNED_SHORT, NULL);
#endif
	
	glUseProgram(0);
	
}

// draw contents of FBO for fullscreen filtering (we have yet to come up with a good one)

void drawFullScreenQuad() {
	
	glBindBuffer(GL_ARRAY_BUFFER, fullscreen_quadData.VBOid);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

	glUseProgram(fullscreen_quad_shader->programHandle());
	glUniform1i(uniform_texture1_loc_fullscreen_quad, 0);
	
	glUseProgram(fullscreen_quad_shader->programHandle());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, FBOtextureid);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glUseProgram(0);

}


void drawWaveVertexArray() {

#ifdef _WIN32

	glBindBuffer(GL_ARRAY_BUFFER, waveVertexArray.VBOid);
	
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

	wave_projection.make_proj_orthographic(-View::zoom, WIN_W+View::zoom, WIN_H+(View::zoom/aspect_ratio), -(View::zoom/aspect_ratio), -1.0f, 1.0f);
	//wave_projection.make_proj_perspective(-zoom, WIN_W+zoom, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), 1.0f, 100.0f);
	
	wave_modelview.identity();

	wave_modelview(3,0) = View::wave_position(0);
	wave_modelview(3,1) = View::wave_position(1);

	glUseProgram(passthrough_shader_program->programHandle());
	glUniform1i(uniform_texture1_loc, 0);
	glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, (const GLfloat*)wave_projection.rawData());
	glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, (const GLfloat*)wave_modelview.rawData());
		
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);

	glDrawArrays(GL_TRIANGLES, 0, BUFSIZE*2-1);

#elif __linux__

	// this code is crap
	glBindBuffer(GL_ARRAY_BUFFER, waveVertexArray.VBOid); 
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glEnableClientState(GL_VERTEX_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));
	
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glUniform1i(uniform_texture1_loc, 0);

	glMatrixMode(GL_PROJECTION);
	
	glOrtho(-zoom*aspect_ratio, WIN_W+zoom*aspect_ratio, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), 0.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(displacement, 0.0, 0.0);

	glUseProgram(programHandle);

	glActiveTexture(GL_TEXTURE0);
	glClientActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);
	glDrawArrays(GL_TRIANGLES, 0, BUFSIZE/2);
//	glPopMatrix();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}

void drawText() {
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	
	glBindBuffer(GL_ARRAY_BUFFER, wpstring_holder::get_static_VBOid());
#ifdef _WIN32
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));
#endif
	
	glUseProgram(passthrough_shader_program->programHandle());
	glUniform1i(uniform_texture1_loc, 0);
	
	glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, (const GLfloat*)Text::projection_matrix.rawData());
	glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, (const GLfloat*)Text::modelview_matrix.rawData());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wpstring_holder::get_IBOid());
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_texture.textureId);

	glDrawElements(GL_TRIANGLES, 6*wpstring_holder::get_static_strings_total_length(), GL_UNSIGNED_SHORT, NULL);
		
	glBindBuffer(GL_ARRAY_BUFFER, wpstring_holder::get_dynamic_VBOid());
	// not sure if needed or not
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

	glUseProgram(passthrough_shader_program->programHandle());
	glUniform1i(uniform_texture1_loc, 0);
	
	glUniformMatrix4fv(uniform_projection_loc, 1, GL_FALSE, (const GLfloat*)Text::projection_matrix.rawData());
	glUniformMatrix4fv(uniform_modelview_loc, 1, GL_FALSE, (const GLfloat*)Text::modelview_matrix.rawData());

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wpstring_holder::get_IBOid());
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, font_texture.textureId);

	glDrawElements(GL_TRIANGLES, 6*wpstring_holder::getDynamicStringCount()*wpstring_max_length, GL_UNSIGNED_SHORT, NULL);

	glUseProgram(0);
	

}


void GetIndices(void* arg) {

	GLuint **p = (GLuint**)arg;
	*p = generateIndexBufferWithSharedVertices();
	_endthread();
}

bool readWAVFile(const std::string& filename) {
	
	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {	
		printf("Couldn't open file %s\n", filename.c_str());
		return false;

	}
	std::size_t num_samples;

	// the readSampleData_int16 function actually reads the whole file.
	float *samples = readSampleData_int16(input, &num_samples);	// presuming signed 16-bit, little endian

	if (num_samples > BUFSIZE_MAX) {
		BUFSIZE=BUFSIZE_MAX;
	} else { BUFSIZE = num_samples; }


	// the bakeWaveVertexBufferUsingLineIntersections is a bit slow...
	// for a 110MB stereo WAV file, it takes ~ 18ms for the stereo->mono
	// downmixing, 20 ms for file I/O, ~32ms for short -> float
	// conversion with scaling, and another whopping 2532ms for bake.
	
	// A solution would be to create an initial vertex buffer object
	// that only includes the first ~four screenfuls of samples,
	// meanwhile creating the final, complete buffer.
	
	Timer::init();
	Timer::start();
	vertex* vertices = bakeWaveVertexBufferUsingLineIntersections(samples, BUFSIZE);
	
	delete [] samples;

	double bake_t = Timer::getMilliSeconds();
	
	printf("Baking took %f ms.\n", bake_t);
	
	waveData.VBOid = generateWaveVertexBufferObject(vertices);	
	
	delete [] vertices;
		
	return true;

}

inline void control() {
	
	// arbitrary timestep
	static const float dt = 0.5;

	if (View::mbuttondown) {
			View::prev_dx = View::dx;
			View::prev_dy = View::dy;

			POINT p;
			GetCursorPos(&p);
				
			//View::dx = -(View::mouse_pos0->x - p.x);
			//View::dy = -(View::mouse_pos0->y - p.y);
			
			View::dx = -(View::prev_mouse_pos->x - p.x);
			View::dy = -(View::prev_mouse_pos->y - p.y);
			//printf("%f, %f\n", View::dx, View::dy);

			const float Ddx = View::dx-View::prev_dx;
			const float Ddy = View::dy-View::prev_dy;

			if (View::dx == 0 && View::dy == 0) {
				View::wave_view_velocity *= 0.1;
			}
			else {
				// with the exp term, the sensitivity now scales with zoom level
				View::wave_view_velocity(0) += (Ddx/dt)*exp(View::zoom/290.0);	
				View::wave_view_velocity(1) += (Ddy/dt)*exp(View::zoom/290.0);
			}
			// in an attempt to make the velocity vector more "sticky"
			View::wave_view_velocity_sample1 = 0.5*(View::wave_view_velocity + View::wave_view_velocity_sample1);

			View::wave_position += View::wave_view_velocity*dt;

			*View::prev_mouse_pos = p;
	}

	else {
		
		View::wave_view_velocity *= 0.88;
		View::wave_view_velocity_sample1 *= 0.88;
		View::wave_position += View::wave_view_velocity_sample1*dt;

	}

}

inline void draw() {
	
	glBindFramebuffer(GL_FRAMEBUFFER, FBOid);
	glClear(GL_COLOR_BUFFER_BIT);
	
	drawWave();
	//drawWaveVertexArray();
	drawText();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	drawFullScreenQuad();
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
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC ;
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
		//dwStyle=WS_OVERLAPPEDWINDOW;
		dwStyle=(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
		// creates a non-resizable window
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



const std::string openFileDialog() {

	OPENFILENAME ofn;

	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = "Microsoft RIFF WAVE files (*.wav)\0*.wav\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "wav";

	if (GetOpenFileName(&ofn)) {
		return std::string(szFileName);
	}

	else { 
		return ""; 
	}

}


LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	switch(uMsg)
	{		

		// actually, the order seems to matter quite a bit
		case WM_LBUTTONUP:
			{
				ClipCursor(NULL);

				ShowCursor(TRUE);	// for some very odd reason, two calls are needed to
				ShowCursor(TRUE);	// accomplish the task :D

				View::mbuttondown=false;
				return 0;
			}
		case WM_LBUTTONDOWN:
			{
				GetWindowRect(hWnd, &View::windowRect);
				ClipCursor(&View::windowRect);
				GetCursorPos(View::mouse_pos0);
				*View::prev_mouse_pos = *View::mouse_pos0;
				View::wave_pos0 = View::wave_position;
				View::mbuttondown = true;
				
				ShowCursor(FALSE);
				ShowCursor(FALSE);
				return 0;
			}

		case WM_MOUSEWHEEL:
			{
				int fwKeys = GET_KEYSTATE_WPARAM(wParam);
				int delta = GET_WHEEL_DELTA_WPARAM(wParam);
				if (delta < 0) {
					View::zoomOut();
				} else if (delta > 0) { View::zoomIn(); }

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

		case WM_CHAR:
			{
				keys[wParam]=TRUE;
				return 0;
			}

		case WM_SIZE:
			{
				return 0;
			}
			;
	
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
				delete View::mouse_pos0;
				PostQuitMessage(0);
				return 0;
			}
			;
	}
	/* the rest shall be passed to defwindowproc. (default window procedure) */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void initializeStrings() {

	// NOTE: it wouldn't be such a bad idea to just take in a vector 
	// of strings, and to generate one single static VBO for them all.

	std::string string1 = "Filename: " + input_filename;
	wpstring_holder::append(wpstring(string1, 15, 15), WPS_DYNAMIC);

	std::string frames("Frames per second: ");
	wpstring_holder::append(wpstring(frames, WIN_W-180, WIN_H-20), WPS_STATIC);

	// reserved index 2 for FPS display.
	std::string initialfps = "00.00";
	wpstring_holder::append(wpstring(initialfps, WIN_W-50, WIN_H-20), WPS_DYNAMIC);
	
	char buf[16];
	sprintf(buf, "%d", BUFSIZE);
	const std::string buffer_size(buf);
	const std::string bufinfostring = "Buffer size / # of samples: " + buffer_size;
	wpstring_holder::append(wpstring(bufinfostring, 15, WIN_H-20), WPS_DYNAMIC);

	const std::string help1("Press 'o' to open a new file.");
	wpstring_holder::append(wpstring(help1, WIN_W-220, 20), WPS_STATIC);
	const std::string help2("'p' for polygonmode toggle.");
	wpstring_holder::append(wpstring(help2, WIN_W-220, 35), WPS_STATIC);
	const std::string help3("'t' for texture toggle.");
	wpstring_holder::append(wpstring(help3, WIN_W-220, 50), WPS_STATIC);

	wpstring_holder::createBufferObjects();

}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

	const std::string cpu_ext_string(checkCPUCapabilities());

	if (cpu_ext_string != "OK") {
		MessageBox(NULL, cpu_ext_string.c_str(), "Fatal error", MB_OK | MB_ICONINFORMATION);
		return EXIT_FAILURE;
	}
	
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

	// start generating index buffer in another thread,
	// since it's in no way related to any of the following 
	// (or preceding) operations

	GLuint *indices = NULL;
	HANDLE handle;

	// the gain is marginal at best, but it's a great drill :P
	handle = (HANDLE) _beginthread(GetIndices, 0, (void*)&indices);
	
	if (!readWAVFile(input_filename)) {
		return 1;
	}


	//float tmpx = 0.0;
///	static float step = 0.125;

	//for (int i=0; i < BUFSIZE; i++)
	//{
	//	lines.push_back(make_line(tmpx, half_WIN_H*(samples[i]+1.0), tmpx + step, half_WIN_H*(samples[i+1]+1.0)));
	//	tmpx+=step;
//	}
	
	initializeStrings();

	//generateWaveVBOs();
	//generateSliderVBOs();
		
	int running=1;

	WaitForSingleObject(handle, INFINITE);

	waveData.IBOid = generateGlobalIndexBuffer(indices);
	
	delete [] indices;

	Timer::init();

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

				// removed keyboard controlling. Refer to github history if you still want that.

				if (keys['o']) {
					// a file dialog is opened :P
					
					const std::string newfilename = openFileDialog();
					if (newfilename != "") {	
						if (input_filename != newfilename) {
							input_filename = newfilename;
							// destroy previous data, open new
							destroyCurrentWaveVertexBuffer();
							if (!readWAVFile(newfilename)) {
								MessageBox(NULL, "Couldn't open file!", "Error!", NULL);
								return 1;
							}
							// update hud
							printf("%s\n", newfilename.c_str());
							// strip file name from absolute path
							const std::size_t newfilename_len = newfilename.length();
							int i = newfilename_len;
							bool done = false;
							while (!done) {
								if (newfilename[--i] == '\\') done = true;
							}			
							const std::string strippedname = newfilename.substr(i+1, newfilename_len-i);

							wpstring_holder::updateDynamicString(0, "Filename: " + strippedname);
							
							char buf[16];
							sprintf(buf, "%d", BUFSIZE);
							const std::string buffer_size(buf);
							const std::string bufinfostring = "Buffer size / # of samples: " + buffer_size;
							
							wpstring_holder::updateDynamicString(2, bufinfostring);

							}
					}
					//printf("%s\n", newfile.c_str());
										
					keys['o'] = false;
				}

				if (keys['p']) {

					// kind of bad, since if done this way,
					// the text will get globbered as well :D
					if (wave_polygonMode == GL_LINE) {
						wave_polygonMode = GL_FILL;
					}
					else { wave_polygonMode = GL_LINE; }

					keys['p'] = false;

				}

				if (keys['t']) {

					wave_solidColorTextureToggle = !wave_solidColorTextureToggle;
					keys['t'] = false;
				}

				control();
				draw(); 
				SwapBuffers(hDC);
	
				double t_interval = Timer::getSeconds();
				
				Timer::start();

				double fps = 1/t_interval;
				char buffer[8];
				sprintf(buffer, "%4.2f", fps);
				std::string fps_str(buffer);
				if (wpstring_holder::getDynamicString(1) != fps_str) {
					wpstring_holder::updateDynamicString(1, buffer);
				}
				

			}
		}

	}

	KillGLWindow();
	glDeleteBuffers(1, &waveData.VBOid);
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
