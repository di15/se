
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
    #define _WIN
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_MAC
        #define _MAC
    #endif
#endif

#pragma warning(disable: 4996)
#pragma warning(disable: 4018)
#pragma warning(disable: 4160)
#pragma warning(disable: 4244)
#pragma warning(disable: 4800)
#pragma warning(disable: 4305)

#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#ifdef _WIN
#include <winsock2.h>	// winsock2 needs to be included before windows.h
#include <windows.h>
#include <mmsystem.h>
#endif

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glaux.h> 
#include <gl\glext.h>

#ifdef _WIN
#include <commdlg.h>
#endif

#include <string>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <list>
#include <set>
#include <fstream>
#include <iostream>
#include <math.h>

#ifdef _WIN
#include <jpeglib.h>
#include <png.h>
#include <zip.h>
#endif

//#include <SDL.h>

#ifdef _WIN
//#pragma comment(lib, "SDL.lib")
//#pragma comment(lib, "SDLmain.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glaux.lib")
#pragma comment(lib, "jpeg.lib")
#pragma comment(lib, "libpng15.lib")
#pragma comment(lib, "zlibstatic.lib")
#pragma comment(lib, "zipstatic.lib")
#endif

using namespace std;

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

/*
#ifndef _isnan
int _isnan(double x) { return x != x; }
#endif
*/

#ifdef _WIN
extern HDC g_hDC;
extern HGLRC g_hRC;
extern HWND g_hWnd;
extern HINSTANCE g_hInstance;
#endif

//#define DEMO
//#define DEBUG
//#define FAST

#endif
