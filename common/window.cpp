
#include "window.h"
#include "texture.h"
#include "draw/shader.h"
#include "gui/gui.h"
#include "gui/font.h"
#include "draw/shadow.h"
#include "math/3dmath.h"
#include "debug.h"

bool g_quit = false;
double g_frameinterval = 0.0f;
int g_width = INI_WIDTH;
int g_height = INI_HEIGHT;
int g_bpp = INI_BPP;
bool g_fullscreen = false;
Resolution g_selectedRes;
vector<Resolution> g_resolution;
vector<int> g_bpps;
Vec2i g_mouse;
Vec2i g_mousestart;
bool g_keyintercepted = false;
bool g_keys[256];
bool g_mousekeys[3];
double g_currentTime;
double g_lastTime = 0.0f;		// This will hold the time from the last frame
double g_framesPerSecond = 0.0f;		// This will store our fps
double g_instantFPS = 0.0f;
long long g_lasttime = GetTickCount();
float g_zoom = 1;

void AddRes(int w, int h)
{
	Resolution r;
	r.width = w;
	r.height = h;
	g_resolution.push_back(r);
}

void EnumerateDisplay()
{
	DEVMODE dm;
	int index=0;
	while(0 != EnumDisplaySettings(NULL, index++, &dm))
	{
		Resolution r;
		r.width = dm.dmPelsWidth;
		r.height = dm.dmPelsHeight;

		bool found = false;

		for(int i=0; i<g_resolution.size(); i++)
		{
			if(g_resolution[i].width == r.width && g_resolution[i].height == r.height)
			{
				found = true;
				break;
			}
		}

		if(!found)
			g_resolution.push_back(r);

		found = false;

		int bpp = dm.dmBitsPerPel;
		
		for(int i=0; i<g_bpps.size(); i++)
		{
			if(g_bpps[i] == bpp)
			{
				found = true;
				break;
			}
		}
		
		if(!found)
			g_bpps.push_back(bpp);
	}
}

void Resize(int width, int height)
{
	if(height == 0)
		height = 1;
	
	glViewport(0, 0, width, height);

	if(g_width != width || g_height != height)
	{
		g_width = width;
		g_height = height;

#ifdef DEBUG
		g_log<<"!=rsz "<<g_width<<","<<g_height<<endl;
		g_log.flush();
#endif

		//if(g_fullscreen)
			//Reload();
			//ReloadTextures();
		g_GUI.resize();
	}
}

void CalculateFrameRate()
{
	static double frameTime = 0.0f;				// This stores the last frame's time

	// Get the current time in seconds
    g_currentTime = timeGetTime() * 0.001f;				

	// We added a small value to the frame interval to account for some video
	// cards (Radeon's) with fast computers falling through the floor without it.

	// Here we store the elapsed time between the current and last frame,
	// then keep the current frame in our static variable for the next frame.
 	g_frameinterval = g_currentTime - frameTime + 0.005f;
	
	//g_instantFPS = 1.0f / (g_currentTime - frameTime);
	//g_instantFPS = 1.0f / g_frameinterval;

	frameTime = g_currentTime;

	// Increase the frame counter
    ++g_framesPerSecond;

	// Now we want to subtract the current time by the last time that was stored
	// to see if the time elapsed has been over a second, which means we found our FPS.
    if( g_currentTime - g_lastTime > 1.0f )
	{
		g_instantFPS = g_framesPerSecond;

		// Here we set the lastTime to the currentTime
	    g_lastTime = g_currentTime;

		// Reset the frames per second
        g_framesPerSecond = 0;
    }
}


bool AnimateNextFrame(int desiredFrameRate)
{
	static double lastTime = GetTickCount() * 0.001;
	static double elapsedTime = 0.0f;

	double currentTime = GetTickCount() * 0.001; // Get the time (milliseconds = seconds * .001)
	double deltaTime = currentTime - lastTime; // Get the slice of time
	double desiredFPS = 1.0 / (double)desiredFrameRate; // Store 1 / desiredFrameRate

	elapsedTime += deltaTime; // Add to the elapsed time
	lastTime = currentTime; // Update lastTime

	// Check if the time since we last checked is greater than our desiredFPS
	if( elapsedTime > desiredFPS )
	{
		elapsedTime -= desiredFPS; // Adjust the elapsed time

		// Return true, to animate the next frame of animation
		return true;
	}

	// We don't animate right now.
	return false;
	/*
	long long currentTime = GetTickCount();
	float desiredFPMS = 1000.0f/(float)desiredFrameRate;
	int deltaTime = currentTime - g_lasttime;

	if(deltaTime >= desiredFPMS)
	{
		g_lasttime = currentTime;
		return true;
	}

	return false;*/
}

bool InitWindow()
{
	glShadeModel(GL_SMOOTH);
	CheckGLError(__FILE__, __LINE__);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	CheckGLError(__FILE__, __LINE__);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	CheckGLError(__FILE__, __LINE__);
	glEnable(GL_DEPTH_TEST);
	CheckGLError(__FILE__, __LINE__);
	glDepthFunc(GL_LEQUAL);
	CheckGLError(__FILE__, __LINE__);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	CheckGLError(__FILE__, __LINE__);
	//glEnable(GL_TEXTURE_2D);
	CheckGLError(__FILE__, __LINE__);
	glEnable(GL_BLEND);
	CheckGLError(__FILE__, __LINE__);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	CheckGLError(__FILE__, __LINE__);
	//glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	CheckGLError(__FILE__, __LINE__);
	glCullFace(GL_BACK);

	CheckGLError(__FILE__, __LINE__);

#if 1
#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
	//glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
   // glClientActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glClientActiveTextureARB");
	
#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif


	if(!glActiveTextureARB 
		//|| !glMultiTexCoord2fARB || !glClientActiveTextureARB
			)
	{
		MessageBox(g_hWnd, TEXT("Your current setup does not support multitexturing"), TEXT("Error"), MB_OK);
		return false;
	}
#endif
	
#ifdef DEBUG
	LastNum(__FILE__, __LINE__);
#endif

	InitGLSL();
	InitShadows();
	LoadFonts();

#if 0
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeofvertdata, tempVertData, GL_STATIC_DRAW);
#endif

	return true;
}

void DestroyWindow(const char* title)
{
	FreeTextures();
	ReleaseShaders();

	if(g_fullscreen)
		ChangeDisplaySettings(NULL, 0);

	if(g_hRC)
	{
		if(!wglMakeCurrent(NULL, NULL))
			MessageBox(NULL, TEXT("Release of DC and RC failed"), TEXT("Error"), MB_OK | MB_ICONINFORMATION);

		if(!wglDeleteContext(g_hRC))
			MessageBox(NULL, TEXT("Release of rendering context failed"), TEXT("Error"), MB_OK | MB_ICONINFORMATION);

		g_hRC = NULL;
	}

	if(g_hDC && !ReleaseDC(g_hWnd, g_hDC))
	{
		MessageBox(NULL, TEXT("Release device context failed"), TEXT("Error"), MB_OK | MB_ICONINFORMATION);
		g_hDC = NULL;
	}

	if(g_hWnd && !DestroyWindow(g_hWnd))
	{
		MessageBox(NULL, TEXT("Could not release hWnd"), TEXT("Error"), MB_OK | MB_ICONINFORMATION);
		g_hWnd = NULL;
	}

	if(!UnregisterClass(title, g_hInstance))
	{
		MessageBox(NULL, TEXT("Could not unregister class"), TEXT("Error"), MB_OK | MB_ICONINFORMATION);
		g_hInstance = NULL;
	}
}

bool MakeWindow(const char* title, HICON icon, WNDPROC wndproc)
{
	unsigned int PixelFormat;
	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;
	RECT WindowRect;
	WindowRect.left=(long)0;
	WindowRect.right=(long)g_selectedRes.width;
	WindowRect.top=(long)0;
	WindowRect.bottom=(long)g_selectedRes.height;

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)wndproc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = g_hInstance;
	wc.hIcon = icon; //LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_GUN));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = title;

	if(!RegisterClass(&wc))
	{
		MessageBox(NULL, TEXT("Failed to register the window class"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	
	if(g_fullscreen)
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= g_selectedRes.width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= g_selectedRes.height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= g_bpp;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			char msg[128];
			wsprintf(msg, TEXT("The requested fullscreen mode %dx%dx%d is not supported by\nyour video card. Use windowed mode instead?"), g_width, g_height, g_bpp);

			if(MessageBox(NULL, msg, title, MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				g_fullscreen = false;
			else
			{
				MessageBox(NULL, TEXT("Program will now close"), TEXT("Error"), MB_OK | MB_ICONSTOP);
				g_quit = true;
				return FALSE;
			}
		}
	}

	int startx = 0;
	int starty = 0;

	if(g_fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WS_OVERLAPPEDWINDOW;
		//startx = CW_USEDEFAULT;
		//starty = CW_USEDEFAULT
		startx = GetSystemMetrics(SM_CXSCREEN)/2 - g_selectedRes.width/2;
		starty = GetSystemMetrics(SM_CYSCREEN)/2 - g_selectedRes.height/2;
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	if (!(g_hWnd = CreateWindowEx(dwExStyle, title, title, dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
								startx, starty,	WindowRect.right-WindowRect.left, WindowRect.bottom-WindowRect.top,
								NULL, NULL,	g_hInstance, NULL)))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Window creation error"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	static PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		g_bpp,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		24,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(g_hDC = GetDC(g_hWnd)))	
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't create a GL device context"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if (!(PixelFormat = ChoosePixelFormat(g_hDC, &pfd)))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't find a suitable pixel format"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if(!SetPixelFormat(g_hDC, PixelFormat, &pfd))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't set the pixel format"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	
	HGLRC tempContext; 
	
	if (!(tempContext = wglCreateContext(g_hDC)))
	//if (!(g_hRC = wglCreateContext(g_hDC)))
	//if (!(g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs)))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't create a GL rendering context"), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	if(!wglMakeCurrent(g_hDC, tempContext))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't activate the GL rendering context"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	int major, minor;
	GetGLVersion(&major, &minor);

	if( major < 3 || ( major == 3 && minor < 2 ) )
	{
		DestroyWindow(title);
		MessageBox(g_hWnd, "OpenGL 3.2 is not supported!", "Error", NULL);
		return false;
	}

#define WGL_CONTEXT_MAJOR_VERSION_ARB       0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB       0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB     0x2093
#define WGL_CONTEXT_FLAGS_ARB           0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB        0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB	0x00000002
	GLint attribs[] =
	{
		// Here we ask for OpenGL 2.1
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 2,
		// Uncomment this for forward compatibility mode
		//WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
		// Uncomment this for Compatibility profile
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		// We are using Core profile here
		//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC) wglGetProcAddress("wglCreateContextAttribsARB");
	if(wglCreateContextAttribsARB == NULL)
	{
		//g_hRC = wglCreateContextAttribsARB(g_hDC,0, attribs);
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't get pointer to wglCreateContextAttribsARB"), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//if (!(tempContext = wglCreateContext(g_hDC)))
	//if (!(g_hRC = wglCreateContext(g_hDC)))
	if (!(g_hRC = wglCreateContextAttribsARB(g_hDC, 0, attribs)))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't create a GL 3.2 rendering context"), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}
	
	wglMakeCurrent(0, 0);
	wglDeleteContext(tempContext);

	if(!wglMakeCurrent(g_hDC, g_hRC))
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Can't activate the GL 3.2 rendering context"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	ShowWindow(g_hWnd, SW_SHOW);
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);
	//Resize(g_width, g_height);

	if(!InitWindow())
	{
		DestroyWindow(title);
		MessageBox(NULL, TEXT("Initialization failed"), TEXT("Error"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}