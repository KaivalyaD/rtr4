// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include"OGL.h"

// OpenGL headers
#include<GL/gl.h>
#include<GL/glu.h>

// math headers
#define _USE_MATH_DEFINES
#include<math.h>

// OpenGL libraries
#pragma comment(lib, "OpenGL32.lib")
#pragma comment(lib, "glu32.lib")

// macros
#define ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED -1
#define ERROR_WIN32_SETPIXELFORMAT_FAILED -2
#define ERROR_WGL_WGLCREATECONTEXT_FAILED -3
#define ERROR_WGL_WGLMAKECURRENT_FAILED -4

#define DEG_TO_RAD(x) ( (x) * ( ( M_PI ) / 180.0f) )

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	int initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	int iRetVal = 0;
	int cxScreen, cyScreen;

	// code
	gpLog = fopen("Log.txt", "w");
	if(!gpLog)
	{
		MessageBox(NULL, TEXT("fopen(): failed to create the log file"), TEXT("CRT Error"), MB_OK | MB_ICONERROR);
		exit(1);
	}
	fprintf(gpLog, "fopen(): successfully created the log file\n");

	// initialization of the WNDCLASSEX structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hInstance = hInstance;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// getting the screen size
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("Concentric Circles: Kaivalya Deshpande"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;

	// initialize GL
	iRetVal = initialize();
	switch(iRetVal)
	{
		case ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED:
			fprintf(gpLog, "ChoosePixelFormat(): failed\n");
			uninitialize();
			break;
		case ERROR_WIN32_SETPIXELFORMAT_FAILED:
			fprintf(gpLog, "SetPixelFormat(): failed\n");
			uninitialize();
			break;
		case ERROR_WGL_WGLCREATECONTEXT_FAILED:
			fprintf(gpLog, "wglCreateContext(): failed\n");
			uninitialize();
			break;
		case ERROR_WGL_WGLMAKECURRENT_FAILED:
			fprintf(gpLog, "wglMakeCurrent(): failed\n");
			uninitialize();
			break;
		default:
			fprintf(gpLog, "OpenGL initialized successfully\n");
			break;
	}
	
	// show the window
	ShowWindow(hwnd, iCmdShow);

	// focus and foregrounding the window
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// game loop
	while(!bDone)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(gbActiveWindow)
			{
				// render the scene
				display();

				// update the scene
				update();
			}
		}
	}

	// uinitialize GL
	uninitialize();

	return static_cast<int>(msg.wParam);
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	void resize(int, int);

	// code
	switch(iMsg)
	{
		case WM_SETFOCUS:
			gbActiveWindow = TRUE;
			fprintf(gpLog, "window in focus\n");
			break;
		case WM_KILLFOCUS:
			gbActiveWindow = FALSE;
			fprintf(gpLog, "window out of focus\n");
			break;
		case WM_ERASEBKGND:
			return 0;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case 27:
					fprintf(gpLog, "destroying after receiving esc\n");
					DestroyWindow(hwnd);
					break;
				default:
					break;
			}
			break;
		case WM_CHAR:
			switch(wParam)
			{
				case 'F':
				case 'f':
					ToggleFullScreen();
					break;
				default:
					break;
			}
			break;
		case WM_SIZE:
			resize(LOWORD(lParam), HIWORD(lParam));
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if(!gbFullScreen)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if(dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if(GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;
			fprintf(gpLog, "fullscreen mode on\n");
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;
		fprintf(gpLog, "fullscreen mode off\n");

		gbFullScreen = FALSE;
	}
}

// GL init
int initialize(void)
{
	// function prototypes
	void resize(int, int);

	// variable declarations
	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex = 0;

	// code
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;
	
	// get the Windows' DC
	ghdc = GetDC(ghwnd);

	// choose format descriptor
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if(!iPixelFormatIndex)
		return ERROR_WIN32_CHOOSEPIXELFORMAT_FAILED;
	
	// set chosen pixel format
	if(!SetPixelFormat(ghdc, iPixelFormatIndex, &pfd))
		return ERROR_WIN32_SETPIXELFORMAT_FAILED;
	
	// create an OpenGL rendering context
	ghrc = wglCreateContext(ghdc);
	if(!ghrc)
		return ERROR_WGL_WGLCREATECONTEXT_FAILED;
	
	if(!wglMakeCurrent(ghdc, ghrc))
		return ERROR_WGL_WGLMAKECURRENT_FAILED;

	// OpenGL begins here
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	// warm-up resize
	resize(WIN_WIDTH, WIN_HEIGHT);

	return 0;
}

void resize(int width, int height)
{
	// code
	if(height == 0)
		height = 1;	// to prevent a divide-by-zero

	glViewport(0.0f, 0.0f, (GLsizei)width, (GLsizei)height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);
}

void display(void)
{
	// variable declarations
	const int countCircles = 10;

	// code
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(0.0f, 0.0f, -3.0f);

	glLineWidth(1.0f);
	for (int i = 0; i < countCircles; i++)
	{
		glBegin(GL_LINE_LOOP);
		{
			switch (i)
			{
			case 0:
				glColor3f(1.0f, 0.0f, 0.0f);
				break;
			case 1:
				glColor3f(0.0f, 1.0f, 0.0f);
				break;
			case 2:
				glColor3f(0.0f, 0.0f, 1.0f);
				break;
			case 3:
				glColor3f(0.0f, 1.0f, 1.0f);
				break;
			case 4:
				glColor3f(1.0f, 0.0f, 1.0f);
				break;
			case 5:
				glColor3f(1.0f, 1.0f, 0.0f);
				break;
			case 6:
				glColor3f(1.0f, 1.0f, 1.0f);
				break;
			case 7:
				glColor3f(0.5f, 0.5f, 0.5f);
				break;
			case 8:
				glColor3f(1.0f, 0.5f, 0.0f);
				break;
			case 9:
				glColor3f(0.5f, 0.0f, 1.0f);
				break;
			default:
				break;
			}
			
			for (int j = 0; j < 360; j++)
				glVertex2f(0.75f * cosf(DEG_TO_RAD(j)), 0.75f * sinf(DEG_TO_RAD(j)));
		}
		glEnd();

		glScalef(0.75f, 0.75f, 0.0f);
	}

	SwapBuffers(ghdc);
}

void update(void)
{
	// code
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if(gbFullScreen)
		ToggleFullScreen();

	if(wglGetCurrentContext() == ghrc)
		wglMakeCurrent(NULL, NULL);
	
	if(ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}
	if(ghdc)
	{
		DeleteDC(ghdc);
		ghdc = NULL;
	}
	if(ghwnd)
	{
		DestroyWindow(ghwnd);
		ghwnd = NULL;
	}
	if(gpLog)
	{
		fprintf(gpLog, "fclose(): closing the log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}
