#ifdef _WIN32
#include <Windows.h>

#include <GL\glew.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include <GL\wglew.h> 

#elif __linux__

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <SDL/SDL.h>

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

#ifdef _WIN32

#define WIN_W 1280 
#define WIN_H 960

#elif __linux__

#define WIN_W 800
#define WIN_H 600

#endif

#define BUFSIZE 65536
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


static float half_WIN_H = (float) WIN_H / 2.0;

static float linewidth = 1.8; 
static float half_linewidth = linewidth/2.0;
static float displacement = 0.0;

static texture gradient_texture, font_texture;

static std::vector<line> lines;
static std::vector<wpstring> strings;

static std::size_t displayrange_left = 0;
static std::size_t displayrange_right = 8*WIN_W;

static GLuint VertexShaderId, FragmentShaderId, programHandle, uniform_texture1_loc;

static bufferObject waveData, text1;

static float zoom = 0.0;

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

	line line;
	double dx = (x2 - x1);
	double dy = (y2 - y1);

	y1 = (WIN_H - y1);
	y2 = (WIN_H - y2);

	double slope = dy/dx;
	float angle = atan(slope);

	float xparm = half_linewidth*sin(angle);	// in order to get the actual, rendered line width to match with the specified one,
	float yparm = -half_linewidth*cos(angle);	// the minus is needed since in OpenGL the y-axis is inverted
				    
	//			    x         y        u    v
	line.vertices[0] = vertex(x1-xparm, y1+yparm, 0.0, 0.0);
	line.vertices[1] = vertex(x1+xparm, y1-yparm, 0.0, 1.0);
	line.vertices[2] = vertex(x2+xparm, y2-yparm, 1.0, 1.0);
	line.vertices[3] = vertex(x2-xparm, y2+yparm, 1.0, 0.0);

	return line;

}




void generateBufferObjects() {

	glGenBuffers(1, &waveData.VBOid);
	glBindBuffer(GL_ARRAY_BUFFER, waveData.VBOid);
	glBufferData(GL_ARRAY_BUFFER, BUFSIZE*sizeof(line), &lines[0], GL_STATIC_DRAW);

	GLuint *indices = new GLuint[(6*BUFSIZE)];	// three indices per triangle, two triangles per line, BUFSIZE lines

	int i = 0;
	int j = 0;
	
	while (i < 6 * BUFSIZE) {

		indices[i] = j;
		indices[i+1] = j+1;
		indices[i+2] = j+3;
		indices[i+3] = j+1;
		indices[i+4] = j+2;
		indices[i+5] = j+3;

		i += 6;
		j += 4;


	}

	glGenBuffers(1, &waveData.IBOid);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waveData.IBOid);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*2*BUFSIZE*(sizeof(GLuint)), indices, GL_STATIC_DRAW);

	delete [] indices;


}



bool InitGL()
{
	
	#ifdef _WIN32
	GLenum err = glewInit();

	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return FALSE;
	}
	#endif

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,WIN_W,WIN_H, 0,0.0f,1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	
	const char* version = (const char*) glGetString(GL_VERSION);

	printf("\nOpenGL version information:\n%s\n\n", version);

	bool gradient_texture_valid = gradient_texture.make_texture("textures/texture.bmp", GL_LINEAR);
	bool font_texture_valid = font_texture.make_texture("textures/dina_all.bmp", GL_NEAREST);

	if (!(gradient_texture_valid && font_texture_valid)) {
		printf("Failure loading textures.");
		return FALSE;
	}

#ifdef _WIN32
	const char* vshadername = "shaders/vertex.shader.win";
	const char* fshadername = "shaders/fragment.shader.win";

#elif __linux__
	const char* vshadername = "shaders/vertex.shader.linux";
	const char* fshadername = "shaders/fragment.shader.linux";

#endif
	
	printf("%s\n", vshadername);
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


	programHandle = glCreateProgram();

    glAttachShader(programHandle, VertexShaderId);
    glAttachShader(programHandle, FragmentShaderId);

    glLinkProgram(programHandle);
    glUseProgram(programHandle);

	delete [] vert_shader;
	delete [] frag_shader;

    if (!checkShader(&FragmentShaderId, GL_COMPILE_STATUS) ||
	    !checkShader(&VertexShaderId, GL_COMPILE_STATUS))
	{
		printf("Shader compile error. See shader.log\n");
		return FALSE;
	}
	
#ifdef _WIN32

	glBindAttribLocation(programHandle, 0, "in_position");
	glBindAttribLocation(programHandle, 1, "in_texcoord");
	glBindFragDataLocation(programHandle, 0, "out_fragcolor");

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

#endif
	uniform_texture1_loc = glGetUniformLocation(programHandle, "texture_1");
	
	text1 = generateTextObject(std::string("1234567890000AEDIOFoifeaoijowaijoiaj fawoeifwe mina olen paskapaa :D:DDDD"), 500, 800);

	return TRUE;
}


void drawLines() {

	glClear(GL_COLOR_BUFFER_BIT);

	
	glBindBuffer(GL_ARRAY_BUFFER, waveData.VBOid);
	
#ifdef _WIN32

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__	// this is because mesa-7 only implements OpenGL up to 1.4
	
	glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));

#endif
	glUniform1i(uniform_texture1_loc, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	static float aspect_ratio = WIN_W/WIN_H;

	glOrtho(-zoom*aspect_ratio, WIN_W+zoom*aspect_ratio, WIN_H+(zoom/aspect_ratio), -(zoom/aspect_ratio), 0.0f, 1.0f);
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(displacement, 0.0, 0.0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waveData.IBOid);
	glUseProgram(programHandle);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gradient_texture.textureId);
	glDrawElements(GL_TRIANGLES, BUFSIZE*2, GL_UNSIGNED_INT, NULL);
	glPopMatrix();
}


void drawText(std::vector<wpstring>& text) {

	std::vector<wpstring>::const_iterator iter = text.begin();

	while(iter != text.end()) {

		glBindBuffer(GL_ARRAY_BUFFER, (*iter).bufObj.VBOid);
#ifdef _WIN32
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(0));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 16, BUFFER_OFFSET(2*sizeof(float)));

#elif __linux__
		glVertexPointer(2, GL_FLOAT, sizeof(vertex), NULL);
		glTexCoordPointer(2, GL_FLOAT, sizeof(vertex), BUFFER_OFFSET(8));
#endif
		glUniform1i(uniform_texture1_loc, 0);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0, WIN_W, WIN_H, 0.0, 0.0, 1.0);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
	
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (*iter).bufObj.IBOid);
		glUseProgram(programHandle);
	
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, font_texture.textureId);

		glDrawElements(GL_TRIANGLES, 6*(*iter).length, GL_UNSIGNED_SHORT, NULL);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		++iter;
	}
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

	const char* filename = "asdfmono.wav";
	strings.push_back(wpstring(std::string("kikkeleita"), 75, 700));
	strings.push_back(wpstring(std::string("perkele"), 50, 100));
	strings.push_back(wpstring(std::string("JUMALAUTASAROSIJAIRJAEOTIrgijsroiawjrg;oairjg;oirajw  "), 500, 100));
	strings.push_back(wpstring(std::string("KAKELI XDD 3003100"), 50, 400));

	std::ifstream input(filename, std::ios::binary);

	if (!input.is_open()) {	
		printf("Couldn't open texture file %s\n", filename);
		return 1;

	}

	float *samples = readSampleData_int16(input);	// presuming 16-bit, little endian

	float tmpx = 0.0;
	static float step = 0.125;

	for (int i=0; i < BUFSIZE; i++)
	{
		lines.push_back(make_line(tmpx, half_WIN_H*samples[i] + WIN_H, tmpx + step, half_WIN_H*samples[i+1] +  WIN_H));
		tmpx+=step;
	}

	delete [] samples;
	
	generateBufferObjects();

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

				drawLines();
				drawText(strings);
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

	float *samples = readSampleData_int16(input);	// presuming 16-bit, little endian

	float tmpx = 0.0;
	static float step = 0.125;

	for (int i=0; i < BUFSIZE; i++)
	{
		lines.push_back(make_line(tmpx, (WIN_H/2)*samples[i] + WIN_H, tmpx + step, (WIN_H/2)*samples[i+1] + WIN_H));
		tmpx+=step;
	}

	delete [] samples;
	
	SDL_Surface *screen = createSDLWindow();
	if (!screen) {
		std::cout << "Couldn't create SDL window.";
		return 1;
	}

	InitGL();

	std::string string1 = std::string("Filename: ") + std::string(filename);
	std::string string2 = std::string("abcdhijklmnopqrstuvxyzABCDEFGHIJLKMNOPQRSTUVXYZ!@#$%^&*()");
	std::string string3 = std::string("opqrstuvwxyz");

	strings.push_back(wpstring(string1, 15, 15));
	strings.push_back(wpstring(string2, 15, 500));
	strings.push_back(wpstring(string3, 15, 550));

	drawLines();
	drawText(strings);
	SDL_GL_SwapBuffers();

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
							drawLines();
							drawText(strings);
							SDL_GL_SwapBuffers();
							break;
						case SDLK_DOWN:
							zoomOut();
							drawLines();
							drawText(strings);
							SDL_GL_SwapBuffers();
							break;
						case SDLK_LEFT:
							translateLeft();
							drawLines();
							drawText(strings);
							SDL_GL_SwapBuffers();
							break;
						case SDLK_RIGHT:
							translateRight();
							drawLines();
							drawText(strings);
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